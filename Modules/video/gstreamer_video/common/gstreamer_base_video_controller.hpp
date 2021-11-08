#pragma once

#include <string>
#include <mutex>
#include <thread>
#include <functional>

#include <cstdio>
#include <cstring>
#include <glib.h>
#include <signal.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef G_OS_UNIX
#include <glib-unix.h>
#include <sys/wait.h>
#elif defined (G_OS_WIN32)
#include <windows.h>
#endif

#include <condition_variable>
#include <cassert>
#include <sstream>


#include <core/video.h>
#include <video/sources/gstreamer_video_source.h>

// RAII wrappers
#include <utils/scope_guard.hpp>
#include <utils/ref_count_ptr.hpp>

// gstreamer includes
#ifdef __GNUC__
    // Avoid tons of warnings with root code
    #pragma GCC system_header
#endif

#include <gst/gst.h>
#include <locale.h>             /* for LC_ALL */
#include <gst/pbutils/missing-plugins.h>

#define GST_API_VERSION "1.0"
#define MAX_INDENT 40

#define N_(String) String

namespace video
{
	namespace common
	{
		static void print_one_tag(const GstTagList * list, const gchar * tag, gpointer user_data)
		{
			int i, num;

			num = gst_tag_list_get_tag_size(list, tag);
			for (i = 0; i < num; ++i) {
				const GValue *val;

				/* Note: when looking for specific tags, use the gst_tag_list_get_xyz() API,
				* we only use the GValue approach here because it is more generic */
				val = gst_tag_list_get_value_index(list, tag, i);
				if (G_VALUE_HOLDS_STRING(val)) {
					g_print("\t%20s : %s\n", tag, g_value_get_string(val));
				}
				else if (G_VALUE_HOLDS_UINT(val)) {
					g_print("\t%20s : %u\n", tag, g_value_get_uint(val));
				}
				else if (G_VALUE_HOLDS_DOUBLE(val)) {
					g_print("\t%20s : %g\n", tag, g_value_get_double(val));
				}
				else if (G_VALUE_HOLDS_BOOLEAN(val)) {
					g_print("\t%20s : %s\n", tag,
						(g_value_get_boolean(val)) ? "true" : "false");
				}
				else if (GST_VALUE_HOLDS_BUFFER(val)) {
					GstBuffer *buf = gst_value_get_buffer(val);
					guint buffer_size = static_cast<guint>(gst_buffer_get_size(buf));

					g_print("\t%20s : buffer of size %u\n", tag, buffer_size);
				}
				else if (GST_VALUE_HOLDS_DATE_TIME(val)) {
					GstDateTime *dt = static_cast<GstDateTime*>(g_value_get_boxed(val));
					gchar *dt_str = gst_date_time_to_iso8601_string(dt);

					g_print("\t%20s : %s\n", tag, dt_str);
					g_free(dt_str);
				}
				else {
					g_print("\t%20s : tag of type '%s'\n", tag, G_VALUE_TYPE_NAME(val));
				}
			}
		}

		static GstBusSyncReply bus_sync_handler(GstBus* bus, GstMessage* message, gpointer data)
		{
			GstElement* pipeline = (GstElement*)data;

			switch (GST_MESSAGE_TYPE(message))
			{
			case GST_MESSAGE_STATE_CHANGED:
				/* we only care about pipeline state change messages */
				if (GST_MESSAGE_SRC(message) == GST_OBJECT_CAST(pipeline))
				{
					GstState old_state, new_state, pending;
					gst_message_parse_state_changed(message, &old_state, &new_state, &pending);
				}
				break;
			default:
				break;
			}

			return GST_BUS_PASS;
		}

		template <typename T>
		class gstreamer_base_video_controller :
			public T
		{
		private:
			enum event_loop_result
			{
				ELR_NO_ERROR = 0,
				ELR_ERROR,
				ELR_INTERRUPT
			};

			core::video::video_state m_state;
			std::mutex m_mutex;
			std::thread m_working_thread;

			gboolean is_live = FALSE;
			gboolean waiting_eos = FALSE;

			// Usually we won't use such declarations as it's not RAII.
			// However, on this specific case and for simplicity reasons,
			// since this memeber is initialized and destructed solely in the working tread,
			// and since the working thread is an RAII managed object,
			// we can set the member as plain pointer
			GstElement* m_pipeline;

			event_loop_result event_loop(GstElement* pipeline, gboolean blocking, gboolean do_progress, GstState target_state)
			{
				event_loop_result retval = ELR_NO_ERROR;
				gboolean buffering = FALSE, in_progress = FALSE;
				gboolean prerolled = target_state != GST_STATE_PAUSED;

				GstBus* bus = gst_element_get_bus(GST_ELEMENT(pipeline));
				utils::scope_guard bus_releaser([&bus]()
				{
					gst_object_unref(bus);
				});

#ifdef G_OS_UNIX
				//	signal_watch_id =
				//		g_unix_signal_add(SIGINT, (GSourceFunc)intr_handler, pipeline);

				//	utils::scope_guard singal_handler_releaser([&]()
				//	{
				//		if (signal_watch_id > 0)
				//			g_source_remove(signal_watch_id);
				//	});
#endif

				while (true)
				{
					GstMessage* message = gst_bus_poll(bus, GST_MESSAGE_ANY, blocking ? -1 : 0);
					GstTagList* tags = nullptr;
					retval = ELR_NO_ERROR;

					/* if the poll timed out, only when !blocking */
					if (message == nullptr)
						return retval;				

					utils::scope_guard messgae_releaser([&]()
					{
						if (tags != nullptr)
						{
							gst_tag_list_unref(tags);
							tags = nullptr;
						}

						gst_message_unref(message);
					});

					switch (GST_MESSAGE_TYPE(message))
					{
					case GST_MESSAGE_NEW_CLOCK:
					{
						break;
					}
					case GST_MESSAGE_CLOCK_LOST:
					{
						// Clock lost, re-clocking
						gst_element_set_state(pipeline, GST_STATE_PAUSED);
						gst_element_set_state(pipeline, GST_STATE_PLAYING);
						break;
					}
					case GST_MESSAGE_EOS:
					{
						waiting_eos = FALSE;
						on_error(video::sources::gstreamer_error_codes::end_of_stream);
						return retval;
					}
					case GST_MESSAGE_TAG:
					{
						/*gst_message_parse_tag(message, &tags);

						g_print("Got tags from element %s:\n", GST_OBJECT_NAME(message->src));
						gst_tag_list_foreach(tags, print_one_tag, NULL);
						g_print("\n");*/

						break;
					}
					case GST_MESSAGE_TOC:
					{
						break;
					}
					case GST_MESSAGE_INFO:
					{
						gchar* name = nullptr;
						GError* error = nullptr;
						gchar* debug = nullptr;

						utils::scope_guard releaser([&]()
						{
							if (debug != nullptr)
								g_free(debug);

							if (error != nullptr)
								g_clear_error(&error);

							if (name != nullptr)
								g_free(name);
						});

						name = gst_object_get_path_string(GST_MESSAGE_SRC(message));
						gst_message_parse_info(message, &error, &debug);

						// TODO: raise siganl (name, error, debug)			
						break;
					}
					case GST_MESSAGE_WARNING:
					{
						gchar* name = nullptr;
						GError* error = nullptr;
						gchar* debug = nullptr;

						utils::scope_guard releaser([&]()
						{
							if (debug != nullptr)
								g_free(debug);

							if (error != nullptr)
								g_clear_error(&error);

							if (name != nullptr)
								g_free(name);
						});

						name = gst_object_get_path_string(GST_MESSAGE_SRC(message));
						gst_message_parse_warning(message, &error, &debug);

						printf("Warning: %s (%s)\n", error->message, debug);
						// TODO: raise siganl (name, error, debug)
						on_error(video::sources::gstreamer_error_codes::stream_warning);
						retval = ELR_ERROR;
						return retval;
					}
					case GST_MESSAGE_ERROR:
					{
						GError* error = nullptr;
						gchar* debug = nullptr;
						utils::scope_guard error_releaser([&error, &debug]()
						{
							if (error != nullptr)
							{
								g_clear_error(&error);
								error = nullptr;
							}

							if (debug != nullptr)
							{
								g_free(debug);
							}
						});

						gst_message_parse_error(message, &error, &debug);
						printf("Error: %s (%s)\n", error->message, debug);
						/* we have an error */

						on_error(video::sources::gstreamer_error_codes::stream_error);
						retval = ELR_ERROR;
						return retval;
					}
					case GST_MESSAGE_STATE_CHANGED:
					{
						GstState old_state, new_state, pending;

						/* we only care about pipeline state change messages */
						if (GST_MESSAGE_SRC(message) != GST_OBJECT_CAST(pipeline))
							break;

						gst_message_parse_state_changed(message, &old_state, &new_state, &pending);

						// TODO: raise signal (oldstate, newstate)

						/* if we reached the final target state, exit */
						if (target_state == GST_STATE_PAUSED && new_state == target_state)
						{
							prerolled = TRUE;

							// ignore when we are buffering since then we mess with the states ourselves
							if (buffering)
							{
								// Prerolled, waiting for buffering to finish...
								break;
							}
							if (in_progress)
							{
								// Prerolled, waiting for progress to finish...
								break;
							}

							return retval;
						}

						// else not an interesting message
						break;
					}
					case GST_MESSAGE_BUFFERING:
					{
						gint percent;
						gst_message_parse_buffering(message, &percent);

						// TODO: raise signal (percent)

						// no state management needed for live pipelines
						if (is_live)
							break;

						if (percent == 100)
						{
							// a 100% message means buffering is done
							buffering = FALSE;

							// if the desired state is playing, go back
							if (target_state == GST_STATE_PLAYING)
							{
								gst_element_set_state(pipeline, GST_STATE_PLAYING);
							}
							else if (prerolled && in_progress == FALSE)
							{
								return retval;
							}
						}
						else
						{
							// buffering busy
							if (buffering == FALSE && target_state == GST_STATE_PLAYING)
							{
								// we were not buffering but PLAYING, PAUSE  the pipeline.
								gst_element_set_state(pipeline, GST_STATE_PAUSED);
							}

							buffering = TRUE;
						}

						break;
					}
					case GST_MESSAGE_LATENCY:
					{
						// Redistribute latency...
						gst_bin_recalculate_latency(GST_BIN(pipeline));
						break;
					}
					case GST_MESSAGE_REQUEST_STATE:
					{
						GstState state;
						gchar* name = nullptr;

						utils::scope_guard releaser([&name]()
						{
							if (name != nullptr)
								g_free(name);
						});

						gst_message_parse_request_state(message, &state);

						// TODO: signal set state request (name, gst_element_state_get_name(state)) - (reqeust, stateval)
						gst_element_set_state(pipeline, state);
						break;
					}
					case GST_MESSAGE_APPLICATION:
					{
						const GstStructure* s = gst_message_get_structure(message);

						if (gst_structure_has_name(s, "GstLaunchStopInterrupt"))
						{
							/* this application message is posted when we need to stop the pipeline. */
							// Interrupt: Stopping pipeline...
							retval = ELR_INTERRUPT;
							return retval;
						}
						break;
					}
					case GST_MESSAGE_PROGRESS:
					{
						GstProgressType type;
						gchar* code = nullptr;
						gchar* text = nullptr;

						utils::scope_guard releaser([&]()
						{
							if (code != nullptr)
								g_free(code);

							if (text != nullptr)
								g_free(text);
						});

						gst_message_parse_progress(message, &type, &code, &text);

						// TODO: raise signal (type, code, text)

						switch (type)
						{
						case GST_PROGRESS_TYPE_START:
						case GST_PROGRESS_TYPE_CONTINUE:
							if (do_progress)
							{
								in_progress = TRUE;
								blocking = TRUE;
							}
							break;
						case GST_PROGRESS_TYPE_COMPLETE:
						case GST_PROGRESS_TYPE_CANCELED:
						case GST_PROGRESS_TYPE_ERROR:
							in_progress = FALSE;
							break;
						default:
							break;
						}

						if (do_progress && in_progress == FALSE && buffering == FALSE && prerolled)
							return retval;
						break;
					}
					case GST_MESSAGE_ELEMENT:
					{
						if (gst_is_missing_plugin_message(message))
						{
							const gchar *desc;
							desc = gst_missing_plugin_message_get_description(message);
							(void)desc;

							// TODO: signal missing plugin (desc)
						}
						break;
					}
					case GST_MESSAGE_HAVE_CONTEXT:
					{
						const gchar* context_type = nullptr;
						GstContext* context = nullptr;
						gchar* context_str = nullptr;

						utils::scope_guard releaser([&]()
						{
							if (context_str != nullptr)
								g_free(context_str);

							if (context != nullptr)
								gst_context_unref(context);
						});

						gst_message_parse_have_context(message, &context);
						context_type = gst_context_get_context_type(context);
						context_str = gst_structure_to_string(gst_context_get_structure(context));

						(void)context_type;

						// TODO: signal have context (context, context_str, context_type)			
						break;
					}
					//		case GST_MESSAGE_PROPERTY_NOTIFY:
					//		{
					//			const GValue* val;
					//			const gchar* name;
					//			GstObject* obj = nullptr;
					//			gchar* obj_name = nullptr;

					//			utils::scope_guard releaser([&]()
					//			{
					//				if (obj_name != nullptr)
					//					g_free(obj_name);

					//				// TODO: check if we need to unref/release obj
					//			});

					//			gst_message_parse_property_notify(message, &obj, &name, &val);
					//			obj_name = gst_object_get_path_string(GST_OBJECT(obj));

					//			// TODO: signal property value changes (obj_name, name, val)

					//			break;
					//		}
					default:
						// Do nothing...
						break;
					}

				}

				// We shouldn't get here...
				g_assert_not_reached();
				return retval;
			}

		protected:
			virtual bool on_start()
			{
				if (m_state == core::video::video_state::PAUSED)
					return on_resume();

				enum init_state
				{
					Unknown,
					Success,
					Failed
				};

				std::mutex mutex;
				std::condition_variable pipeline_creation_sync;
				init_state start_state = init_state::Unknown;

				m_working_thread = std::thread([&]()
				{
					init_state local_state = start_state;
					utils::scope_guard notify_on_failure([&]()
					{
						if (local_state == init_state::Unknown)
						{
							std::unique_lock<std::mutex> locker(mutex);
							local_state = start_state = init_state::Failed;
							locker.unlock();

							pipeline_creation_sync.notify_one();
						}
					});

					gst_init(nullptr, nullptr);

#ifdef G_OS_UNIX
					//        if (!no_fault)
					//            fault_setup();
#endif			

					GError* error = nullptr;
					utils::scope_guard error_releaser([&error]()
					{
						if (error != nullptr)
						{
							g_clear_error(&error);
							error = nullptr;
						}
					});

					// Safety reasons to make sure we're not messing up with threads...
					assert(m_pipeline == nullptr);

					std::string pipeline_string(pipeline_description());
					printf("GStreamer Pipline: %s\n", pipeline_string.c_str());
					if (pipeline_string.empty() == true)
						throw std::runtime_error("Empty pipeline description");

					m_pipeline = (GstElement*)gst_parse_launch(pipeline_string.c_str(), &error);
					if (m_pipeline == nullptr)
						throw std::runtime_error("Pipeline creation failed");

					utils::scope_guard pipeline_releaser([&]()
					{
						if (m_pipeline != nullptr)
						{
							// Setting pipeline to NULL state ...
							gst_element_set_state(m_pipeline, GST_STATE_NULL);

							// Freeing pipeline ...
							gst_object_unref(m_pipeline);

							m_pipeline = nullptr;
							on_pipeline_destroyed();
						}						
					});

					/* If the top-level object is not a pipeline, place it in a pipeline. */
					if (GST_IS_PIPELINE(m_pipeline) == false)
					{
						GstElement* real_pipeline = gst_element_factory_make("pipeline", "real_pipeline");
						if (real_pipeline == nullptr)
							return -1;

						gst_bin_add(GST_BIN(real_pipeline), m_pipeline);
						m_pipeline = real_pipeline;
					}

					std::unique_lock<std::mutex> locker(mutex);
					local_state = start_state = init_state::Success;
					locker.unlock();
					pipeline_creation_sync.notify_one();

					on_pipeline_constructed(m_pipeline);				

					/* options */
					gboolean eos_on_shutdown = FALSE;
					gulong deep_notify_id = 0;
					event_loop_result retval = event_loop_result::ELR_NO_ERROR;
					
					GstBus* bus = gst_element_get_bus(m_pipeline);
					utils::scope_guard bus_releaser([&bus]()
					{
						if (bus == nullptr)
							return;

						gst_bus_remove_signal_watch(bus);
						gst_bus_set_sync_handler(bus, nullptr, nullptr, nullptr);
						gst_object_unref(bus);
						bus = nullptr;
					});

					if (bus != nullptr)
					{
						// set the bus sync handler
						gst_bus_set_sync_handler(bus, bus_sync_handler, (gpointer)m_pipeline, nullptr);
						gst_bus_add_signal_watch(bus);
					}

					// Setting pipeline to PAUSED ...			
					GstStateChangeReturn ret = gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
					switch (ret)
					{
					case GST_STATE_CHANGE_FAILURE:
						// ERROR: Pipeline doesn't want to pause.
						event_loop(m_pipeline, FALSE, FALSE, GST_STATE_VOID_PENDING);
						return -1;
					case GST_STATE_CHANGE_NO_PREROLL:
						// Pipeline is live and does not need PREROLL ...
						is_live = TRUE;
						break;
					case GST_STATE_CHANGE_ASYNC:
						// Pipeline is PREROLLING ...
						retval = event_loop(m_pipeline, FALSE, TRUE, GST_STATE_PAUSED);
						if (retval != event_loop_result::ELR_NO_ERROR)
						{
							// ERROR: pipeline doesn't want to preroll...
							return static_cast<int>(retval);
						}
						/* fallthrough */
					case GST_STATE_CHANGE_SUCCESS:
						// Pipeline is PREROLLED ...
						break;
					}

					retval = event_loop(m_pipeline, FALSE, TRUE, GST_STATE_PLAYING);
					if (retval != event_loop_result::ELR_NO_ERROR)
					{
						// ERROR: pipeline doesn't want to preroll.
						return static_cast<int>(retval);
					}

					// Setting pipeline to PLAYING ...
					if (gst_element_set_state(m_pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
						return -1;

					// Playback loop...
					retval = event_loop(m_pipeline, TRUE, FALSE, GST_STATE_PLAYING);

					on_pipeline_completed();

					// Wait for eos...
					if (eos_on_shutdown && retval != ELR_NO_ERROR)
					{
						gboolean ignore_errors;
						waiting_eos = TRUE;

						if (retval == ELR_INTERRUPT)
						{
							// EOS on shutdown enabled -- Forcing EOS on the pipeline
							gst_element_send_event(m_pipeline, gst_event_new_eos());
							ignore_errors = FALSE;
						}
						else
						{
							// EOS on shutdown enabled -- waiting for EOS after Error
							ignore_errors = TRUE;
						}

						// Waiting for EOS...
						while (TRUE)
						{
							retval = event_loop(m_pipeline, TRUE, FALSE, GST_STATE_PLAYING);

							if (retval == ELR_NO_ERROR)
							{
								/* we got EOS */
								// EOS received - stopping pipeline...
								break;
							}
							else if (retval == ELR_INTERRUPT)
							{
								// Interrupt while waiting for EOS - stopping pipeline...
								break;
							}
							else if (retval == ELR_ERROR)
							{
								if (ignore_errors == FALSE)
								{
									// An error happened while waiting for EOS
									break;
								}
							}
						}
					}

					// Setting pipeline to PAUSED ...
					GstState state, pending;
					gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
					if (retval == ELR_NO_ERROR)
						gst_element_get_state(m_pipeline, &state, &pending, GST_CLOCK_TIME_NONE);

					/* iterate mainloop to process pending stuff */
					while (g_main_context_iteration(nullptr, FALSE));

					/* No need to see all those pad caps going to NULL etc., it's just noise */
					if (deep_notify_id != 0)
						g_signal_handler_disconnect(m_pipeline, deep_notify_id);

					// Setting pipeline to READY ...
					gst_element_set_state(m_pipeline, GST_STATE_READY);
					gst_element_get_state(m_pipeline, &state, &pending, GST_CLOCK_TIME_NONE);

					return static_cast<int>(retval);
				});

				std::unique_lock<std::mutex> locker(mutex);
				pipeline_creation_sync.wait(locker, [&]()
				{
					return (start_state != init_state::Unknown);
				});
				locker.unlock();

				if (start_state == init_state::Success)
					m_state = core::video::video_state::PLAYING;

				return (m_state == core::video::video_state::PLAYING);
			}

			virtual bool on_resume()
			{
				GstState state, pending;
				gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
				gst_element_get_state(m_pipeline, &state, &pending, GST_CLOCK_TIME_NONE);
				if (state == GstState::GST_STATE_PLAYING)
					m_state = core::video::video_state::PLAYING;

				return (m_state == core::video::video_state::PLAYING);
			}

			virtual void on_stop()
			{
				m_state = core::video::video_state::STOPPED;

				if (m_pipeline != nullptr)
				{
					// Signal the pipeline to stop...
					gst_element_post_message(GST_ELEMENT(m_pipeline),
						gst_message_new_application(GST_OBJECT(m_pipeline),
							gst_structure_new("GstLaunchStopInterrupt",
								"message", G_TYPE_STRING, "Pipeline interrupted by Stop request", NULL)));
				}
				
				if (m_working_thread.joinable() == true)
				{
					if (std::this_thread::get_id() != m_working_thread.get_id())
						m_working_thread.join();
					else
						m_working_thread.detach();
				}				
			}

			virtual void on_pause()
			{
				GstState state, pending;
				gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
				gst_element_get_state(m_pipeline, &state, &pending, GST_CLOCK_TIME_NONE);

				if (state == GstState::GST_STATE_PAUSED)
					m_state = core::video::video_state::PAUSED;
			}

			virtual std::string pipeline_description() = 0;

			virtual void on_pipeline_constructed(GstElement* pipeline)
			{
			}

			virtual void on_pipeline_completed()
			{
			}

			virtual void on_pipeline_destroyed()
			{
			}

			virtual void on_error(int error_code)
			{
			}

		public:
			gstreamer_base_video_controller() :
				m_state(core::video::video_state::STOPPED), m_pipeline(nullptr)
			{
			}

			virtual ~gstreamer_base_video_controller()
			{
				stop();
			}

			inline virtual core::video::video_state state() override
			{
				return m_state;
			}

			virtual void start() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);

				if (m_state != core::video::video_state::PLAYING)
					on_start();
			}

			virtual void stop() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				if (m_state != core::video::video_state::STOPPED)
					on_stop();
			}

			virtual void pause() override
			{
				std::lock_guard<std::mutex> locker(m_mutex);
				if (m_state == core::video::video_state::PLAYING)
					on_pause();
			}
		};		
	}
}

