#include <bits/types/struct_timeval.h>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

std::string getTime()
{
	time_t   timeStamp = time(NULL);
	std::tm *dateTime = std::gmtime(&timeStamp);
	mktime(dateTime);
	std::string timeStr;

	// Week day
	switch (dateTime->tm_wday - 1) // -1 offset because wtf
	{
	case 0:
		timeStr += "Mon";
		break;
	case 1:
		timeStr += "Tue";
		break;
	case 2:
		timeStr += "Wed";
		break;
	case 3:
		timeStr += "Thu";
		break;
	case 4:
		timeStr += "Fri";
		break;
	case 5:
		timeStr += "Sat";
		break;
	case 6:
		timeStr += "Sun";
		break;
	}
	timeStr += ", ";

	std::ostringstream sstream;

	// Month day
	sstream << dateTime->tm_mday;
	timeStr += sstream.str() + " ";
	sstream.str("");

	// Month
	switch (dateTime->tm_mon)
	{
	case 0:
		timeStr += "Jan";
		break;
	case 1:
		timeStr += "Feb";
		break;
	case 2:
		timeStr += "Mar";
		break;
	case 3:
		timeStr += "Apr";
		break;
	case 4:
		timeStr += "May";
		break;
	case 5:
		timeStr += "Jun";
		break;
	case 6:
		timeStr += "Jul";
		break;
	case 7:
		timeStr += "Aug";
		break;
	case 8:
		timeStr += "Sep";
		break;
	case 9:
		timeStr += "Oct";
		break;
	case 10:
		timeStr += "Nov";
		break;
	case 11:
		timeStr += "Dec";
		break;
	}
	timeStr += " ";

	// Year
	sstream << dateTime->tm_year + 1900 << " ";
	// Hour
	sstream << dateTime->tm_hour << ":" << dateTime->tm_min << ":" << dateTime->tm_sec << " GMT";
	timeStr += sstream.str();

	return timeStr;
}

int main(void)
{
	std::cout << getTime() << std::endl;
}
