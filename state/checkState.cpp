#include <string>
#include <algorithm> 
#include <cctype>
#include <iostream>
#include <locale>
#include <sstream>
#include <vector>
#include <math.h>
#include <fstream>
#include <filesystem>
#include <time.h>
#include <glib.h>
#include <librdkafka/rdkafka.h>
using namespace std;
clock_t g_start_t;
clock_t g_end_t;
string stdformat(const char* fmt, ...) {
	int size = 2048;
	char* buffer = new char[size];
	va_list vl;
	va_start(vl, fmt);
	int nsize = vsnprintf(buffer, size, fmt, vl);
	if (size <= nsize) { //fail delete buffer and try again
		delete[] buffer;
		buffer = 0;
		buffer = new char[nsize + 1]; //+1 for /0
		nsize = vsnprintf(buffer, size, fmt, vl);
	}
	string ret(buffer);
	va_end(vl);
	delete[] buffer;
	return ret;
}
/* Wrapper to set config values and error out if needed.
 */
static void set_config(rd_kafka_conf_t *conf, char *key, char *value) {
    char errstr[512];
    rd_kafka_conf_res_t res;

    res = rd_kafka_conf_set(conf, key, value, errstr, sizeof(errstr));
    if (res != RD_KAFKA_CONF_OK) {
        g_error("Unable to set config: %s", errstr);
        exit(1);
    }
}
/* Optional per-message delivery callback (triggered by poll() or flush())
 * when a message has been successfully delivered or permanently
 * failed delivery (after retries).
 */
static void dr_msg_cb (rd_kafka_t *kafka_handle,
                       const rd_kafka_message_t *rkmessage,
                       void *opaque) {
    if (rkmessage->err) {
        g_error("Message delivery failed: %s", rd_kafka_err2str(rkmessage->err));
    }
}
static volatile sig_atomic_t run = 1;
static void stop(int sig) {
    run = 0;
}
int main (int argc, char **argv) {
	g_start_t = clock();
	string topic = stdformat("%s",argv[1]);
	/////////////////////////////////////
	// 진행상태 토픽 컨슈머
	/////////////////////////////////////	
    rd_kafka_t *consumer;
    rd_kafka_conf_t *conf;
    rd_kafka_resp_err_t err;
    char errstr[512];

    // Create client configuration
    conf = rd_kafka_conf_new();

    // User-specific properties that you must set
    //set_config(conf, "bootstrap.servers", "192.168.1.115:29092,192.168.1.115:39092,192.168.1.115:49092");
	set_config(conf, "bootstrap.servers", "218.235.89.19:29092,218.235.89.19:39092,218.235.89.19:49092");
    set_config(conf, "group.id",          "group_state"); //컨슈머 그룹 ID
    set_config(conf, "auto.offset.reset", "earliest"); //earliest:먼저 들어온 메세지 부터 처리,latest : 마지막 메세지 부터 처리

    // Create the Consumer instance.
    consumer = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, sizeof(errstr));
    if (!consumer) {
        g_error("Failed to create new consumer: %s", errstr);
        return 1;
    }
    rd_kafka_poll_set_consumer(consumer);

    // Configuration object is now owned, and freed, by the rd_kafka_t instance.
    conf = NULL;

    // Convert the list of topics to a format suitable for librdkafka.
    rd_kafka_topic_partition_list_t *subscription = rd_kafka_topic_partition_list_new(1);
    rd_kafka_topic_partition_list_add(subscription, topic.c_str(), RD_KAFKA_PARTITION_UA);

    // Subscribe to the list of topics.
    err = rd_kafka_subscribe(consumer, subscription);
    if (err) {
        g_error("Failed to subscribe to %d topics: %s", subscription->cnt, rd_kafka_err2str(err));
        rd_kafka_topic_partition_list_destroy(subscription);
        rd_kafka_destroy(consumer);
        return 1;
    }

    rd_kafka_topic_partition_list_destroy(subscription);

    // Install a signal handler for clean shutdown.
    signal(SIGINT, stop);
	
    // Start polling for messages.
	unsigned long cnt_p1,cnt_p2,cnt_p3,cnt_p4,cnt_p5,cnt_p6;
	cnt_p1=cnt_p2=cnt_p3=cnt_p4=cnt_p5=cnt_p6=0;
	unsigned long total_count=0;
    while (run) {
        rd_kafka_message_t *consumer_message;

        consumer_message = rd_kafka_consumer_poll(consumer, 500);
        if (!consumer_message) {
            g_message("Waiting...");
			g_end_t = clock();
			long sec = (long)((g_end_t - g_start_t) / CLOCKS_PER_SEC);
			printf("%d min %d sec\n",sec/60, sec%60);
            continue;
        }

        if (consumer_message->err) {
            if (consumer_message->err == RD_KAFKA_RESP_ERR__PARTITION_EOF) {
                /* We can ignore this error - it just means we've read
                 * everything and are waiting for more data.
                 */
            } else {
                g_message("Consumer error: %s", rd_kafka_message_errstr(consumer_message));
                return 1;
            }
        } else {
            g_message("Consumed event from topic %s: key = %.*s value = %s",
                      rd_kafka_topic_name(consumer_message->rkt),
                      (int)consumer_message->key_len,
                      (char *)consumer_message->key,
                      (char *)consumer_message->payload
                      );
			//0. key
			string key = stdformat("%s",consumer_message->key);
			//1. get count
			string retStr = stdformat("%s",consumer_message->payload);
			if(key.compare(string("tc"))==0) {
				total_count=atoi(retStr.c_str());
			}
			else if(key.compare(string("c"))==0) count++;
			if(total_count==count) {
				print("total count completed\n");
				rd_kafka_message_destroy(consumer_message);
				break;
			}
			g_end_t = clock();
			long sec = (long)((g_end_t - g_start_t) / CLOCKS_PER_SEC);
			printf("%d min %d sec\n",sec/60, sec%60);
			printf("count - %d\n",count);
			// Free the message when we're done.
			rd_kafka_message_destroy(consumer_message);
        }
    }//while end
	
    // Close the consumer: commit final offsets and leave the group.
    g_message( "Closing consumer");
    rd_kafka_consumer_close(consumer);

    // Destroy the consumer.
    rd_kafka_destroy(consumer);	
	return 0;
}