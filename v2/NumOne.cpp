#include <iostream>
#include <string>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <algorithm>

class Buff 
{	
	std::string buff;
public:
	std::mutex lockmx;
	std::condition_variable cv;
	std::string status;
	Buff(std::string str);
	Buff() {};
	~Buff() {};
	std::string ReadBuffer();
	void SetBuffer(std::string str);
	void ThreadOneFunc();
	void ThreadTwoFunc();
};

bool isNumeric(std::string const& str);


int main(int args, char* argv[])
{
	Buff strBuff("");
	std::thread threadOne(&Buff::ThreadOneFunc,std::ref(strBuff));
	std::thread threadTwo(&Buff::ThreadTwoFunc,std::ref(strBuff));

	threadTwo.join();
	threadOne.join();
	
	system("pause");
}


bool isNumeric(std::string const& str)
{
    return !str.empty() && str.find_first_not_of("0123456789") == std::string::npos;
}

Buff::Buff(std::string str) :buff(str), status("avaliable") {}

std::string Buff::ReadBuffer()
{
    return buff;
}

void Buff::SetBuffer(std::string str)
{
    buff = str;
}

void Buff::ThreadOneFunc()
{
    while (true) {
        std::unique_lock<std::mutex> ulFirst(lockmx);
        cv.wait(ulFirst, [=]()
            {
                return status == "avaliable";
            });
        status = "blocked";
        std::string input;
        std::string temp = "";
        std::cout << "\nInput data:\n";
        std::cin >> input;
        if (isNumeric(input) && input.length() < 65)
        {
            std::sort(input.begin(), input.end(), std::greater<char>());
            std::cout << "Data entered." << std::endl;
            for (size_t i = 0; i < input.length(); i++)
            {
                if (input[i] % 2 != 0)
                {
                    temp += input[i];
                }
                else
                {
                    temp += "KB";
                }
            }
            SetBuffer(temp);
            status = "avaliable";
            cv.notify_one();
            cv.wait(ulFirst);
        }
        else
        {
            std::cout << "Entered invalid data. Try again." << std::endl;
            status = "avaliable";
        }
    }
}

//I am a student and I do not have much expetience of working with sockets
//If you are reading this message, please try to launch both programs
//almost at the same time(first program, and then second). I really want to 
//get an internship and learn programming even more.

void Buff::ThreadTwoFunc()
{
    struct sockaddr_in addr;
    socklen_t sizeOfAddr;

    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(1703);
    addr.sin_family = AF_INET;

    while (true) {
    	int flag;
        int Connection = socket(AF_INET, SOCK_STREAM, NULL);
        flag = connect(Connection, (sockaddr*)&addr, sizeof(addr));
        if ( flag != 0)
        {
            cv.notify_one();
            sleep(2);
            continue;
        }
        else if(flag == 0)
        {
            std::cout<<"\nConnection is set. Input Data: \n";
            while (true) {
                std::unique_lock<std::mutex> ulSecond(lockmx);
                cv.wait(ulSecond, [=]()
                    {
                        return status == "avaliable";
                    });
                status = "blocked";
                std::string countable = ReadBuffer();
                if (countable.empty())
                {
                    status == "avaliable";
                    cv.wait(ulSecond);
                    continue;
                }
                std::cout <<"Result: "<< countable << std::endl;
                std::string numReference = "0123456789";
                int sumOfDigits = 0;
                for (int i = 0; i < countable.size(); i++)
                {
                    if (numReference.find(countable[i]) != -1)
                    {
                        sumOfDigits += static_cast<int>(countable[i] - '0');
                    }
                }
                const std::string message = std::to_string(sumOfDigits);
                char* mes = new char[3];
                if (message.size() == 1)
                {
                    mes[0] = message[0];
                    mes[1] = -1;
                    mes[2] = -1;
                }
                else if (message.size() == 2)
                {
                    mes[0] = message[0];
                    mes[1] = message[1];
                    mes[2] = -1;
                }
                else
                {
                    mes[0] = message[0];
                    mes[1] = message[1];
                    mes[2] = message[2];
                }
                int sendValue = send(Connection, mes, sizeof(mes), MSG_NOSIGNAL);
                if ( sendValue == -1 )
                {
                    delete[] mes;
                    status = "avaliable";
                    SetBuffer("");
                    cv.notify_one();
                    cv.wait(ulSecond);
                    break;
                }
                else if(sendValue==8)
                {
                    delete[] mes;
                    status = "avaliable";
                    SetBuffer("");
                    cv.notify_one();
                    cv.wait(ulSecond);
                }
                else
                {
                    delete[] mes;
                    status = "avaliable";
                    SetBuffer("");
                    cv.notify_one();
                    cv.wait(ulSecond);
                } 
            }
        }
    }
}
