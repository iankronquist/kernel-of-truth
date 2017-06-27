#include <truth/log.h>
#include <truth/posix/sys/time.h>
#include <truth/types.h>
#include <arch/x64/port.h>


enum cmos_register {
    CMOS_Command = 0x70,
    CMOS_Data = 0x71,
};


enum cmos_command {
    CMOS_Seconds = 0x00,
    CMOS_Minutes = 0x02,
    CMOS_Hours = 0x04,
    CMOS_Weekday = 0x06,
    CMOS_Day_Of_Month =0x07,
    CMOS_Month = 0x08,
    CMOS_Year = 0x09,
    CMOS_Century = 0x32,
    CMOS_Status_A = 0x0a,
    CMOS_Status_B = 0x0b,
};


#define minutes_to_seconds(x) ((x) * 60)
#define hours_to_seconds(x) (minutes_to_seconds((x) * 60))
#define days_to_seconds(x) (hours_to_seconds((x) * 24))
#define standard_years_to_seconds(x) (days_to_seconds(365 * x))
#define leap_days_since_1970(x) ((((x) - 1972) / 4) - (((x) - 1970) / 100))

#define CMOS_Convert_BCD(x) (((x) & 0x0f) + ((x) / 16 * 10))
#define CMOS_Convert_Hours_BCD(x) ((((x) & 0x0f) + (((x) & 0x70) / 16 * 10)) |\
                                    ((x) & 0x80))
#define CMOS_Format_BCD 0x04
#define CMOS_Format_24_Hour 0x02
#define CMOS_Hour_PM_Bit 0x80
#define CMOS_Hour_Digits 0x7f


uint8_t CMOS_Days_Per_Standard_Month[13] = {
    0,  // Months are one indexed
    31,
    28, // Feb. Leap days handled separately
    31,
    30,
    31,
    30,
    31,
    31,
    30,
    31,
    30,
    31,
};


uint8_t cmos_read(enum cmos_command command) {
    write_port(command, CMOS_Command);
    return read_port(CMOS_Data);
}


void cmos_get_time_of_day(struct timeval *time) {
    uint8_t seconds1, century, year_low, month, day_of_month, hours, minutes,
            seconds2, format;

    do {
        seconds1 = cmos_read(CMOS_Seconds);
        century = cmos_read(CMOS_Century);
        year_low = cmos_read(CMOS_Year);
        month = cmos_read(CMOS_Month);
        day_of_month = cmos_read(CMOS_Day_Of_Month);
        hours = cmos_read(CMOS_Hours);
        minutes = cmos_read(CMOS_Minutes);
        seconds2 = cmos_read(CMOS_Seconds);
    } while (seconds1 != seconds2);

    format = cmos_read(CMOS_Status_B);

    if (!(format & CMOS_Format_BCD)) {
        century = CMOS_Convert_BCD(century);
        year_low = CMOS_Convert_BCD(year_low);
        month = CMOS_Convert_BCD(month);
        day_of_month = CMOS_Convert_BCD(day_of_month);
        hours = CMOS_Convert_Hours_BCD(hours);
        minutes = CMOS_Convert_BCD(minutes);
        seconds1 = CMOS_Convert_BCD(seconds1);
    }

    if (!(format & CMOS_Format_24_Hour) && (hours & CMOS_Hour_PM_Bit)) {
        hours = ((hours & CMOS_Hour_Digits) + 12) % 24;
    }

    uint64_t year = century * 100 + year_low;

    uint64_t epoch_time = seconds1 +
                          minutes_to_seconds(minutes) +
                          hours_to_seconds(hours) +
                          days_to_seconds(day_of_month) +
                          days_to_seconds(leap_days_since_1970(year)) +
                          days_to_seconds(
                                  CMOS_Days_Per_Standard_Month[month]) +
                          standard_years_to_seconds(year_low);
    logf(Log_Debug, "Time: %d/%d/%ld %d:%d:%d\n", day_of_month, month, year,
            hours, minutes, seconds1);
    time->tv_sec = epoch_time;
    time->tv_usec = 0;
}


int gettimeofday(struct timeval *restrict tp, void *restrict tzp) {
    struct timezone *tv = tzp;
    tv->tz_minuteswest = 0;
    tv->tz_dsttime = 0;
    cmos_get_time_of_day(tp);
    return 0;
}


int settimeofday(const struct timeval *unused(tp),
                 const struct timezone *unused(tzp)) {
    // FIXME
    return -1;
}
