#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <rainman/rainman.h>
#include <hzip/api/api.h>
#include <hzip/api/handlers/socket_class.h>
#include <hzip/api/api_enums.h>
#include <hzip/errors/api.h>

using namespace hzapi;

class ApiTestEnvironment : public testing::Environment {
public:
    static void launchApi() {
        static bool hasApiLaunched = false;

        if (hasApiLaunched) {
            return;
        } else {
            hasApiLaunched = true;
        }

        std::thread([]() {
            auto api = hz_api();
            auto mgr = new rainman::memmgr;

            rinitfrom(mgr, api);


            api.limit(1)
                    ->process(1)
                    ->protect("hybridzip")
                    ->timeout(timeval{.tv_sec=5, .tv_usec=0})
                    ->start("127.0.0.1", 1729);
        }).detach();

        usleep(100000);
    }

    void SetUp() override { }
};

class ApiTest : public testing::Test {
protected:
    void SetUp() override {
        ApiTestEnvironment::launchApi();
    }

    static void tsend(int sock, const void *buf, size_t n) {
        if (send(sock, buf, n, 0) < n) {
            throw ApiErrors::ConnectionError("Insufficient data sent");
        }

        if (errno != 0) {
            throw ApiErrors::ConnectionError("Send operation failed");
        }
    }

    static void trecv(int sock, void *buf, size_t n) {
        if (recv(sock, buf, n, 0) < n) {
            throw ApiErrors::ConnectionError("Insufficient data received");
        }

        if (errno != 0) {
            throw ApiErrors::ConnectionError("Receive operation failed");
        }
    }

    static void handshake(int sock) {
        uint64_t token;
        trecv(sock, &token, sizeof(token));
        std::string pass = "hybridzip";

        int n = pass.length() - 1;
        char *y = (char *) &token;

        while (n >= 0) {
            y[n & 0x7] ^= pass[n];
            n--;
        }

        tsend(sock, &token, sizeof(token));

        uint8_t word;

        trecv(sock, &word, sizeof(word));

        ASSERT_EQ(word, COMMON_CTL_SUCCESS);

        uint64_t len;
        trecv(sock, &len, sizeof(len));

        char *msg = new char[len];
        trecv(sock, msg, len);
    }

    static int get_connection() {
        int sock = socket(PF_INET, SOCK_STREAM, 0);
        sockaddr_in server_addr{};

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(1729);

        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

        socklen_t addr_size = sizeof(server_addr);
        if (connect(sock, (sockaddr *) &server_addr, addr_size) < 0) {
            EXPECT_TRUE(false);
        }

        printf("connected socket: %d\n", sock);
        handshake(sock);

        return sock;
    }

    static void end(int sock) {
        uint8_t word = API_CTL_CLOSE;
        send(sock, &word, sizeof(word), 0);
    }
};

TEST_F(ApiTest, hzip_api_test_handshake) {
    int sock = get_connection();
    end(sock);
}

TEST_F(ApiTest, hzip_api_test_stream_1) {
    int sock = get_connection();
    end(sock);
}