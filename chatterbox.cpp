#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>

using namespace std;

int main (int argc, char** argv) {
	if (argv[1] == NULL) {
		cout << "\033[31mTARGET NOT SPECIFIED - TERMINATING...\033[0m\n";
		return -1;
	}
	if (argv[2] == NULL) {
		cout << "\033[31mPORT NOT SPECIFIED - TERMINATING...\033[0m\n";
		return -2;
	}
	
	string target = argv[1];
	int port = atoi(argv[2]);

	cout << "GENERATING SOCKET...\n";
	int chatter = socket(AF_INET, SOCK_STREAM, 0);
	if (chatter == -1) {
		cout << "\033[31mSOCKET GENERATION FAILURE - TERMINATING...\033[0m\n";
		return -3;
	}
	cout << "\033[32mSUCCESSFULLY GENERATED SOCKET\033[0m\n";

	struct sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port   = htons(port);
	inet_pton(AF_INET, target.c_str(), &hint.sin_addr);

	int recval;

	cout << "CONNECTING TO " << target << " AT PORT " << port << "...\n";	
	int connection_status = connect(chatter, (sockaddr*)&hint, sizeof(hint));
	if (connection_status == -1) {
		cout << "\033[31mCONNECTION FAILURE - TERMINATING...\033[0m\n";
		return -4;
	}
	cout << "\033[32mCONNECTED TO HOST\033[0m\n";

	char buf[4096] = {0};
	string msg;
	while (true) {
		struct timeval tv;
		tv.tv_usec = 0.0;
		tv.tv_sec = 5;

		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(chatter, &rfds);

		getline(cin, msg);
		msg += "\r\n";
		const char *pMsg = msg.c_str();
		size_t msgSize = msg.size();
		do {
			int numSent = send(chatter, pMsg, msgSize, 0);
			if (numSent == -1) {
				cout << "\033[31mMESSAGE SENDING FAILURE - TERMINATING...\033[0m\n";
				close(chatter);
				return -5;
			}
			pMsg += numSent;
			msgSize -= numSent;
		} while (msgSize > 0);

		recval = select(chatter + 1, &rfds, NULL, NULL, &tv);
		switch(recval) {
			case(0):
				cout << "\033[31mTIMEOUT\033[0m\n";
				break;
			case(-1):
				cout << "\033[31mERROR\033[0m\n";
				break;
			default:
				int numRead = recv(chatter, buf, 4096, 0);
				if (numRead < 0) {
					cout << "\033[31mFAILURE TO RECEIVE MESSAGE - TERMINATING...\033[0m\n";
					close(chatter);
					return -6;
				}
				else if (numRead == 0) {
					cout << "\033[31mDISCONNECTED - TERMINATING...\033[0m\n";
					close(chatter);
					break;
				} else {
					cout << numRead << "\n";
					cout.write(buf, numRead);
					cout << "\n";
				}
				break;
		}
	}

	close(chatter);

	return 0;
}
