#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <rainman/rainman.h>
#include <hzip/api/api.h>
#include <hzip/api/handlers/socket_class.h>
#include <hzip/api/api_enums.h>

using namespace hzapi;

class ApiTestEnvironment : public testing::Environment {
public:
    // Assume there's only going to be a single instance of this class, so we can just
    // hold the timestamp as a const static local variable and expose it through a
    // static member function


    static bool launchApi() {
        static bool hasApiLaunched = false;
        bool z = hasApiLaunched;
        hasApiLaunched = true;

        return z;
    }

    // Initialise the timestamp in the environment setup.
    void SetUp() override { }
};

class ApiTest : public testing::Test {
protected:
    void SetUp() override {
        if (ApiTestEnvironment::launchApi()) {
            return;
        }

        std::thread([]() {
            auto api = hz_api();
            auto mgr = new rainman::memmgr;

            rinitfrom(mgr, api);


            api.limit(1)
                    ->process(1)
                    ->protect("hybridzip")
                    ->timeout(timeval{.tv_sec=120, .tv_usec=0})
                    ->start("127.0.0.1", 1729);
        }).detach();

        usleep(100000);
    }

    static void handshake(int sock) {
        uint64_t token;
        recv(sock, &token, sizeof(token), 0);
        std::string pass = "hybridzip";

        int n = pass.length() - 1;
        char *y = (char *) &token;

        while (n >= 0) {
            y[n & 0x7] ^= pass[n];
            n--;
        }

        send(sock, &token, sizeof(token), 0);
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

        handshake(sock);

        uint8_t word;
        recv(sock, &word, sizeof(word), 0);

        if (word != COMMON_CTL_SUCCESS) {
            throw std::exception();
        }

        return sock;
    }

    static void end(int sock) {
        uint8_t word = API_CTL_CLOSE;
        send(sock, &word, sizeof(word), 0);
    }
};

TEST_F(ApiTest, hzip_api_test_handshake) {
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

    handshake(sock);

    uint8_t word;

    recv(sock, &word, sizeof(word), 0);
    ASSERT_EQ(word, COMMON_CTL_SUCCESS);

    end(sock);
}

TEST_F(ApiTest, hzip_api_test_stream_1) {
    int sock = get_connection();

    end(sock);
}