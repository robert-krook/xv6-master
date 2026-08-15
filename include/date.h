struct cmos_time_struct 
{
	int	sec;
    int min;
	int	hour;
	int	dow;				// day of week - we don't use this since
								// many cmos chips don't keep this updated
	int	day;
	int	month;
	int	year;
	int	stat_a;
	int	stat_b;
};
