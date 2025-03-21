INC_ROOT = /home/hadoop/project/jsyun/kafka/XDConvertImage/state
#라이브러리 헤러파일과 라이브러리 파일 위치 확인
#CFLAGS=-Wall $(shell pkg-config --cflags glib-2.0 rdkafka)
#LDLIBS=$(shell pkg-config --libs glib-2.0 rdkafka)

CFLAGS=-I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include
#LDLIBS는 예약어라 기본적인 라이브러 폴더를 조사함, 그래서 위치를 설정 안해도 됨.
LDLIBS=-lglib-2.0 -lrdkafka -lpthread -ldl
#USERLIBS는 사용자 지정은 위치를 설정해야함.
USERLIBS= -L/home/hadoop/project/jsyun/kafka/XDConvertImage/state

CC = g++
COMPLGCC = $(CC) -Wall -g -lstdc++fs -std=c++17 -fpermissive $(CFLAGS) -c $<

OBJS_MAIN = checkState.o

OBJS = $(OBJS_MAIN)

# objes 파일 이용해서 실행파일 만들기, 라이브러리 있는 폴더
all:$(OBJS)
	$(CC) -g -o ./checkState $(OBJS) $(LDLIBS) $(USERLIBS)
clean:
	rm $(INC_ROOT)/*.o 
################################
## OBJS_MAIN
################################
checkState.o:$(INC_ROOT)/checkState.cpp
		$(COMPLGCC)
