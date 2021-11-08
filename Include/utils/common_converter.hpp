
#pragma once
#include <cmath>

namespace common_converter
{
	struct time_nanosec_resolution
	{
		time_nanosec_resolution() :
			m_seconds(0),
			m_nanoseconds(0)
		{}

		time_nanosec_resolution(long i_seconds, int  i_nanoseconds) :
			m_seconds(i_seconds),
			m_nanoseconds(i_nanoseconds)
		{}

		long m_seconds;
		int  m_nanoseconds;
	};

	class unit_convertion
	{
	public:
		static constexpr double SAMPALE_RATE = 0.05;
		static constexpr double GPS_START_SEC_DIFF_FROM_EPOCH = 315964800;  ///different second from 1.1.1970 (EPOCH time start) to 6.1.1980 GPS time start
		static constexpr double SEC_IN_WEEK = 604800;     ///number of seconds in a week
		static constexpr double PI = 3.14159265358979323846;
		static constexpr int	PRECISION = 5;
		static constexpr double ACCELERATION_SCALE_FACTOR = (0.2 / 65536) / 125;
		static constexpr double GYROSCOPE_SCALE_FACTOR = (0.008 / 65536) / 125;


		struct T_DateTimeType
		{
			int64_t m_A_second_;
			int32_t m_A_nanoseconds_;

			T_DateTimeType() :
				m_A_second_(0),
				m_A_nanoseconds_(0)
			{}

			T_DateTimeType(int64_t i_A_second_, int32_t i_A_nanoseconds_) :
				m_A_second_(i_A_second_),
				m_A_nanoseconds_(i_A_nanoseconds_)
			{}
		};


		static inline void split_double(double d, int &integer_part, int &fractional_part)
		{
			integer_part = static_cast<int>(d);
			fractional_part = static_cast<int>((d - integer_part) * std::pow(10, PRECISION) + 0.5);
		}

		static inline void week_sectime_to_double(uint32_t week, double seconds, double &timeStamp)
		{
			timeStamp = GPS_START_SEC_DIFF_FROM_EPOCH + week * SEC_IN_WEEK + seconds;
		}


		static inline void GPS_time_to_DDS_time(uint16_t week, uint32_t milli, T_DateTimeType &t_date_time_type)
		{
			//convert the milli to seconds and save the remainder milliseconds
			double seconds = milli / 1000;
			int32_t nanoseconds = (milli%(1000))*1000000;

			t_date_time_type.m_A_second_ = static_cast<int64_t>(GPS_START_SEC_DIFF_FROM_EPOCH + static_cast<double>(week) * SEC_IN_WEEK + seconds);
			t_date_time_type.m_A_nanoseconds_ = nanoseconds;
		}





		static inline void week_sectime_to_nano(uint32_t week, double seconds, time_nanosec_resolution &timeStamp)
		{
			double timeDouble = 0;
			week_sectime_to_double(week, seconds, timeDouble);

			timeStamp.m_seconds = static_cast<long>(timeDouble);
			timeStamp.m_nanoseconds = static_cast<int>((timeDouble - static_cast<double>(timeStamp.m_seconds)) * pow(10, 9));    ///convert seconds to nano seconds
		}


        static inline void converter_sec_and_nano_to_double(int64_t sec, int32_t nano, double &timeStamp)
        {
            timeStamp = static_cast<double>(sec) + nano/1e9;
        }


		static inline double degrees_to_radians(double degrees)
		{
			double radians;
			radians = degrees * (PI / 180);
			return radians;
		}

		static inline double radians_to_degrees(double radian)
		{
			double degrees;
			degrees = radian * (180 / PI);
			return degrees;
		}
	};



} /// namespace CommonConverter

