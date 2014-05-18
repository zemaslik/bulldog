#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdarg.h>
#include "serial.h"

void errorMessage(char* messageFormat, ...) {
	va_list args;
	va_start(args, messageFormat);
	fprintf (stderr, messageFormat, args);
	va_end(args);
}

int serialAttributes(int fd, int speed, int parity, int readTimeout) {
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if (tcgetattr(fd, &tty) != 0) {
		errorMessage("error %d from tcgetattr", errno);
		return -1;
	}

	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         		  // disable break processing
	tty.c_lflag = 0;                		  // no signaling chars, no echo,
											  // no canonical processing
	tty.c_oflag = 0;                		  // no remapping, no delays
	tty.c_cc[VMIN] = 0;             		  // read doesn't block
	tty.c_cc[VTIME] = readTimeout;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY);   // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);		 // ignore modem controls,
											 // enable reading
	tty.c_cflag &= ~(PARENB | PARODD);       // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		errorMessage("error %d from tcsetattr", errno);
		return -1;
	}

	return 0;
}

void serialBlocking(int fd, int block, int readTimeout) {
	struct termios tty;
	memset(&tty, 0, sizeof tty);

	if (tcgetattr(fd, &tty) != 0) {
		errorMessage("error %d from tggetattr", errno);
		return;
	}

	tty.c_cc[VMIN]  = block;
	tty.c_cc[VTIME] = readTimeout;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		errorMessage("error %d setting term attributes", errno);
	}
}

int serialOpen(char* portname, int baud, int parity, int blocking, int readTimeout) {
	int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) {
		errorMessage("error %d opening %s: %s", errno, portname, strerror(errno));
		return 1;
	}

	serialAttributes(fd, baud, parity, readTimeout);
	serialBlocking(fd, blocking, readTimeout);

	return 0;
}

int serialOpenSimple(char* portname, int baud) {
	return serialOpen(portname, baud, 0, SERIAL_NO_BLOCK, SERIAL_DEFAULT_TIMEOUT);
}

int serialClose(int fd) {
	return close(fd);
}

int serialWrite(int fileDescriptor, unsigned char data) {
	return write(fileDescriptor, &data, 1);
}

int serialWriteBuffer(int fileDescriptor, char* data) {
	return write(fileDescriptor, data, strlen(data));
}

int serialReadBuffer(int fileDescriptor, char* buffer, int bufferSize) {
	return read(fileDescriptor, buffer, bufferSize);
}

unsigned char serialReadCharacter(int fileDescriptor) {
	char output;
	if(read(fileDescriptor, &output, 1) < 0) {

	}

	return output;
}

int serialDataAvailable(int fd) {
  int result;

  if (ioctl(fd, FIONREAD, &result) == -1) {
	  return -1;
  }

  return result;
}


