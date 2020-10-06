#include <gtest/gtest.h>
#include <rainman/rainman.h>
#include <hzip/api/api.h>
#include <hzip/api/handlers/socket_class.h>
#include <arpa/inet.h>

class ApiTest : public testing::Test {
protected:
    void SetUp() override {
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
};

TEST_F(ApiTest, hzip_api_test_handshake) {
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{};

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1729);

    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    socklen_t addr_size = sizeof(server_addr);
    if (connect(sock, (sockaddr *)&server_addr, addr_size) < 0) {
        EXPECT_TRUE(false);
    }

    handshake(sock);

    uint8_t word;

    recv(sock, &word, sizeof(word), 0);
    ASSERT_EQ(word, COMMON_CTL_SUCCESS);
}