# light_clock
Modified Light Up Clock for Kids

This is a modified software for the "$10 Light Up Clock for Kids" designed by JonathanT (see the instructables article, http://www.instructables.com/id/Green-Means-Go-10-Light-Up-Clock-for-Kids/). It allows setting of the real time clock and a seven day schedule when connected to the USB port of a computer.

On Windows, a serial terminal program, such as PuTTY (https://www.chiark.greenend.org.uk/~sgtatham/putty/) is required when setting the real time clock or modifying the schedule. When the light up clock is connected to the PC for the first time, it will install a USB serial driver (USB-SERIAL CH340) and associate a serial port (e.g. COM2) with it. The port can be found by looking in the Device Manager under "Ports (COM & LPT)."  In PuTTY, the serial connection should be to the indicated port at 9600 baud, 8-bit, 1 stop-bit, no parity, with local echo, and a line feed with carriage return. On connection, PuTTY will receive the message:

Waiting 10 seconds to enter command mode

If the program receives any character (followed by a line feed), it will enter command mode and print what it thinks is the current time and prompt for changing the time. For example,

 6/4/2018 (Monday) 14:44:26   
Change system date/time (y/n)?

If the program receives "y" it will prompt for the new time.

Enter date and time (MM/DD/YYYY-HH:MN)? 

The user enters the time in the format indicated and the program responds with the new time. For example,

06/04/2018-13:45   
 6/4/2018 (Monday) 13:45:00

The program then prints out the current seven-day schedule and prompts for changing it (Note that the following is the default schedule).

<code>
Day             Red On  Yellow On       Green On        Green Off   
Sunday          6:00    6:50            7:00            7:15   
Monday          6:00    6:50            7:00            7:15   
Tuesday         6:00    6:50            7:00            7:15   
Wednesday       6:00    6:50            7:00            7:15   
Thursday        6:00    6:50            7:00            7:15   
Friday          6:00    6:50            7:00            7:15   
Saturday        6:00    6:50            7:00            7:15   </code>
Change Light Schedule (y/n/d)?

If the program receives "y" it will issue prompts to change a particular day's schedule. For example,

Enter a day of the week to change (0=Sun, 1=Mon, etc)?   
1
Enter on times for red, yellow, and green lights and off time for green light in minutes (HH:MN-HH:MN-HH:MN-HH:MN)?   
14:50-14:51-14:52-14:53   
<code>
Day             Red On  Yellow On       Green On        Green Off   
Sunday          6:00    6:50            7:00            7:15   
Monday          14:50   14:51           14:52           14:53   
Tuesday         6:00    6:50            7:00            7:15   
Wednesday       6:00    6:50            7:00            7:15   
Thursday        6:00    6:50            7:00            7:15   
Friday          6:00    6:50            7:00            7:15   
Saturday        6:00    6:50            7:00            7:15   </code>
Change Light Schedule (y/n/d)? n

If the program receives "d" it will set the schedule to the default. Upon receiving "n" the program will leave command mode and enter normal mode with the message:

Starting clock
