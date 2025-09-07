#include "Grove_lib.h"

#define SPI_CS(x)                 \
    do                            \
    {                             \
        if (_cs >= 0)             \
            gpio_put(_cs, x); \
    } while (0)
 
SSCMA::SSCMA()
{
    _i2c = NULL;
    _address = I2C_ADDRESS;
    _wait_delay = 2;
    _rst = -1;
    _cs = -1;
    _sync = -1;
    tx_len = 0;
    rx_len = 0;
}
 
SSCMA::~SSCMA() {}
 
bool SSCMA::begin(i2c_inst_t *i2c, int32_t rst, uint16_t address, uint32_t wait_delay,
                   uint32_t clock)
{
    _rst = rst;
    _i2c = i2c;
    _address = address;
    _wait_delay = wait_delay;

    i2c_init(i2c, 100 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    set_rx_buffer(SSCMA_MAX_RX_SIZE);
    set_tx_buffer(SSCMA_MAX_TX_SIZE);
 
    if (response != NULL) {
        cJSON_Delete(response);
    }
    response = cJSON_CreateObject();
 
    if (_rst >= 0)
    {
        gpio_init(_rst);
        gpio_set_dir(_rst, GPIO_OUT);
        gpio_put(_rst, 0);
        sleep_ms(50);
        gpio_put(_rst, 1);
        sleep_ms(500);
    }

    return ID(false) && name(false);
}

int SSCMA::write(const char *data, int length)
{
        return i2c_write(data, length);
}
 
int SSCMA::read(char *data, int length)
{
        return i2c_read(data, length);
}
 
int SSCMA::available()
{
        return i2c_available();
}
 
void SSCMA::i2c_cmd(uint8_t feature, uint8_t cmd, uint16_t len, uint8_t *data)
{
    sleep_ms(_wait_delay);
    uint8_t buf[64];
    int idx = 0;
    buf[idx++] = feature;
    buf[idx++] = cmd;
    buf[idx++] = (len >> 8) & 0xFF;
    buf[idx++] = len & 0xFF;

    if (data != nullptr) {
        memcpy(&buf[idx], data, len);
        idx += len;
    }

    // TODO checksum
    buf[idx++] = 0;
    buf[idx++] = 0;
    i2c_write_blocking(_i2c, _address, buf, idx, false);
}


int SSCMA::i2c_available()
{
    uint8_t cmd[6] = {
        FEATURE_TRANSPORT,
        FEATURE_TRANSPORT_CMD_AVAILABLE,
        0, 0,  // payload length = 0
        0, 0   // checksum
    };

    uint8_t buf[2] = {0};

    int ret1 = i2c_write_blocking(_i2c, _address, cmd, sizeof(cmd), false);
    if (ret1 < 0) {
        printf("Write failed (ret1=%d)\n", ret1);
        return -1;
    }

    sleep_ms(10);

    int ret = i2c_read_blocking(_i2c, _address, buf, 2, false);
    if (ret < 0) {
        printf("Read failed (ret=%d)\n", ret);
        return -1;
    }

    printf("Available: 0x%02X 0x%02X (%d)\n", buf[0], buf[1], (buf[0] << 8) | buf[1]);
    return (buf[0] << 8) | buf[1];
}



int SSCMA::i2c_read(char *data, int length)
{
    uint16_t packets = length / MAX_PL_LEN;
    uint8_t remain = length % MAX_PL_LEN;

    for (uint16_t i = 0; i < packets; i++) {
        sleep_ms(_wait_delay);

        uint8_t cmd[6] = {
            FEATURE_TRANSPORT, FEATURE_TRANSPORT_CMD_READ,
            (uint8_t)(MAX_PL_LEN >> 8), (uint8_t)(MAX_PL_LEN & 0xFF),
            0, 0
        };

        if (i2c_write_blocking(_i2c, _address, cmd, sizeof(cmd), false) >= 0) {
            sleep_ms(_wait_delay);
            i2c_read_blocking(_i2c, _address, (uint8_t *)(data + i * MAX_PL_LEN), MAX_PL_LEN, false);
        }
    }

    if (remain) {
        sleep_ms(_wait_delay);

        uint8_t cmd[6] = {
            FEATURE_TRANSPORT, FEATURE_TRANSPORT_CMD_READ,
            (uint8_t)(remain >> 8), (uint8_t)(remain & 0xFF),
            0, 0
        };

        if (i2c_write_blocking(_i2c, _address, cmd, sizeof(cmd), false) >= 0) {
            sleep_ms(_wait_delay);
            i2c_read_blocking(_i2c, _address, (uint8_t *)(data + packets * MAX_PL_LEN), remain, false);
        }
    }

    return length;
}
 
int SSCMA::i2c_write(const char *data, int length)
{
    uint16_t packets = length / MAX_PL_LEN;
    uint16_t remain = length % MAX_PL_LEN;

    for (uint16_t i = 0; i < packets; i++) {
        sleep_ms(_wait_delay);

        uint8_t buf[6 + MAX_PL_LEN];
        buf[0] = FEATURE_TRANSPORT;
        buf[1] = FEATURE_TRANSPORT_CMD_WRITE;
        buf[2] = (MAX_PL_LEN >> 8);
        buf[3] = (MAX_PL_LEN & 0xFF);
        memcpy(&buf[4], data + i * MAX_PL_LEN, MAX_PL_LEN);
        buf[4 + MAX_PL_LEN] = 0; // checksum
        buf[5 + MAX_PL_LEN] = 0;

        i2c_write_blocking(_i2c, _address, buf, 6 + MAX_PL_LEN, false);
    }

    if (remain) {
        sleep_ms(_wait_delay);

        uint8_t buf[6 + MAX_PL_LEN];
        buf[0] = FEATURE_TRANSPORT;
        buf[1] = FEATURE_TRANSPORT_CMD_WRITE;
        buf[2] = (remain >> 8);
        buf[3] = (remain & 0xFF);
        memcpy(&buf[4], data + packets * MAX_PL_LEN, remain);
        buf[4 + remain] = 0;
        buf[5 + remain] = 0;

        i2c_write_blocking(_i2c, _address, buf, 6 + remain, false);
    }

    return length;
}

void SSCMA::praser_event(cJSON *root)
{
    cJSON *name = cJSON_GetObjectItem(root, "name");
    if (!name || !strstr(name->valuestring, CMD_AT_INVOKE))
        return;

    cJSON *data = cJSON_GetObjectItem(root, "data");
    if (!data) return;

    // perf
    cJSON *perf = cJSON_GetObjectItem(data, "perf");
    if (perf && cJSON_GetArraySize(perf) >= 3)
    {
        _perf.prepocess   = cJSON_GetArrayItem(perf, 0)->valueint;
        _perf.inference   = cJSON_GetArrayItem(perf, 1)->valueint;
        _perf.postprocess = cJSON_GetArrayItem(perf, 2)->valueint;
    }

    // boxes
    cJSON *boxes = cJSON_GetObjectItem(data, "boxes");
    if (boxes && cJSON_IsArray(boxes))
    {
        _boxes.clear();
        cJSON *box;
        cJSON_ArrayForEach(box, boxes)
        {
            if (cJSON_GetArraySize(box) >= 6)
            {
                boxes_t b;
                b.x = cJSON_GetArrayItem(box, 0)->valueint;
                b.y = cJSON_GetArrayItem(box, 1)->valueint;
                b.w = cJSON_GetArrayItem(box, 2)->valueint;
                b.h = cJSON_GetArrayItem(box, 3)->valueint;
                b.score = cJSON_GetArrayItem(box, 4)->valuedouble;
                b.target = cJSON_GetArrayItem(box, 5)->valueint;
                _boxes.push_back(b);
            }
        }
    }

    // classes
    cJSON *classes = cJSON_GetObjectItem(data, "classes");
    if (classes && cJSON_IsArray(classes))
    {
        _classes.clear();
        cJSON *cls;
        cJSON_ArrayForEach(cls, classes)
        {
            if (cJSON_GetArraySize(cls) >= 2)
            {
                classes_t c;
                c.score  = cJSON_GetArrayItem(cls, 0)->valuedouble;
                c.target = cJSON_GetArrayItem(cls, 1)->valueint;
                _classes.push_back(c);
            }
        }
    }

    // points
    cJSON *points = cJSON_GetObjectItem(data, "points");
    if (points && cJSON_IsArray(points))
    {
        _points.clear();
        cJSON *pt;
        cJSON_ArrayForEach(pt, points)
        {
            if (cJSON_GetArraySize(pt) >= 4)
            {
                point_t p;
                p.x      = cJSON_GetArrayItem(pt, 0)->valueint;
                p.y      = cJSON_GetArrayItem(pt, 1)->valueint;
                p.score  = cJSON_GetArrayItem(pt, 2)->valuedouble;
                p.target = cJSON_GetArrayItem(pt, 3)->valueint;
                _points.push_back(p);
            }
        }
    }

    // keypoints
    cJSON *keypoints = cJSON_GetObjectItem(data, "keypoints");
    if (keypoints && cJSON_IsArray(keypoints))
    {
        _keypoints.clear();
        cJSON *kp;
        cJSON_ArrayForEach(kp, keypoints)
        {
            if (cJSON_GetArraySize(kp) >= 2)
            {
                keypoints_t k;
                cJSON *box = cJSON_GetArrayItem(kp, 0);
                cJSON *pts = cJSON_GetArrayItem(kp, 1);

                if (box && cJSON_GetArraySize(box) >= 6)
                {
                    k.box.x      = cJSON_GetArrayItem(box, 0)->valueint;
                    k.box.y      = cJSON_GetArrayItem(box, 1)->valueint;
                    k.box.w      = cJSON_GetArrayItem(box, 2)->valueint;
                    k.box.h      = cJSON_GetArrayItem(box, 3)->valueint;
                    k.box.score  = cJSON_GetArrayItem(box, 4)->valuedouble;
                    k.box.target = cJSON_GetArrayItem(box, 5)->valueint;
                }

                if (pts && cJSON_IsArray(pts))
                {
                    cJSON *pt;
                    cJSON_ArrayForEach(pt, pts)
                    {
                        if (cJSON_GetArraySize(pt) >= 4)
                        {
                            point_t p;
                            p.x      = cJSON_GetArrayItem(pt, 0)->valueint;
                            p.y      = cJSON_GetArrayItem(pt, 1)->valueint;
                            p.score  = cJSON_GetArrayItem(pt, 2)->valuedouble;
                            p.target = cJSON_GetArrayItem(pt, 3)->valueint;
                            k.points.push_back(p);
                        }
                    }
                }
                _keypoints.push_back(k);
            }
        }
    }

    cJSON *image = cJSON_GetObjectItem(data, "image");
    if (image && cJSON_IsString(image))
    {
        _image = std::string(image->valuestring);
    }
}

void SSCMA::praser_log()
{
}
 
int SSCMA::wait(int type, const char *cmd, uint32_t timeout)
{
    int ret = CMD_OK;
    unsigned long startTime = to_ms_since_boot(get_absolute_time());
    while (to_ms_since_boot(get_absolute_time()) - startTime <= timeout)
    {
        int len = available();
        if (len == 0)
            continue;
        if (len + rx_end > this->rx_len)
        {
            len = this->rx_len - rx_end;
            if (len <= 0)
            {
                rx_end = 0;
                continue;
            }
        }

        rx_end += read(rx_buf + rx_end, len);
        rx_buf[rx_end] = '\0';

        while (char *suffix = strnstr(rx_buf, RESPONSE_SUFFIX, rx_end))
        {
            if (char *prefix = strnstr(rx_buf, RESPONSE_PREFIX, suffix - rx_buf))
            {
                // get json payload
                len = suffix - prefix + RESPONSE_SUFFIX_LEN;
                payload = (char *)malloc(len);

                if (!payload)
                {
                    continue;
                }

                memcpy(payload, prefix + 1, len - 1); // remove "\r" and "\n"
                memmove(rx_buf, suffix + RESPONSE_SUFFIX_LEN, rx_end - (suffix - rx_buf) - RESPONSE_SUFFIX_LEN);
                rx_end -= suffix - rx_buf + RESPONSE_SUFFIX_LEN;
                payload[len - 1] = '\0';
                
                printf("[Payload]:\n%s\n", payload);
                // parse json
                cJSON *root = cJSON_Parse(payload);
                free(payload);
                if (!root)
                {
                    continue;
                }

                cJSON *typeItem = cJSON_GetObjectItem(root, "type");
                cJSON *nameItem = cJSON_GetObjectItem(root, "name");
                cJSON *codeItem = cJSON_GetObjectItem(root, "code");
 
                if (typeItem && typeItem->valueint == CMD_TYPE_EVENT)
                {
                    praser_event(root);
                }

                if (typeItem && typeItem->valueint == CMD_TYPE_LOG)
                {
                    praser_log();
                }

                if (codeItem)
                    ret = codeItem->valueint;

                if (typeItem && typeItem->valueint == type &&
                    nameItem && strncmp(nameItem->valuestring, cmd, strlen(cmd)) == 0)
                {
                    cJSON_Delete(root);
                    return ret;
                }
            }
            else
            {
                // discard this reply
                memmove(rx_buf, suffix + RESPONSE_PREFIX_LEN, rx_end - (suffix - rx_buf) - RESPONSE_PREFIX_LEN);
                rx_end -= suffix - rx_buf + RESPONSE_PREFIX_LEN;
                rx_buf[rx_end] = '\0';
            }
        }
        sleep_ms(_wait_delay);
    }
    return CMD_ETIMEDOUT;
}

int SSCMA::invoke(int times, bool filter, bool show)
{
    char cmd[64] = {0};

    if (show && rx_len < 16 * 1024)
    {
        return CMD_ENOTSUP;
    }

    snprintf(cmd, sizeof(cmd), CMD_PREFIX "%s=%d,%d,%d" CMD_SUFFIX,
            CMD_AT_INVOKE, times, !filter, filter); // AT+INVOKE=1,0,1\r\n
    write(cmd, strlen(cmd));

    if (wait(CMD_TYPE_RESPONSE, CMD_AT_INVOKE) == CMD_OK)
    {
        if (wait(CMD_TYPE_EVENT, CMD_AT_INVOKE) == CMD_OK)
        {
            return CMD_OK;
        }
    }

    return CMD_ETIMEDOUT;
}

char *SSCMA::ID(bool cache)
{
    if (cache && _ID)
    {
        return _ID;
    }
    char cmd[64] = {0};

    snprintf(cmd, sizeof(cmd), CMD_PREFIX "%s" CMD_SUFFIX, CMD_AT_ID);

    write(cmd, strlen(cmd));

    if (wait(CMD_TYPE_RESPONSE, CMD_AT_ID) == CMD_OK)
    {
        strcpy(_ID, cJSON_GetObjectItem(response, "data")->valuestring);
        return _ID;
    }

    return NULL;
}

char *SSCMA::name(bool cache)
{
    if (cache && _name[0])
    {
        return _name;
    }
    char cmd[64] = {0};
    snprintf(cmd, sizeof(cmd), CMD_PREFIX "%s" CMD_SUFFIX, CMD_AT_NAME);

    write(cmd, strlen(cmd));

    if (wait(CMD_TYPE_RESPONSE, CMD_AT_NAME, 3000) == CMD_OK)
    {
        strcpy(_name, cJSON_GetObjectItem(response, "data")->valuestring);
        return _name;
    }

    return NULL;
}

bool SSCMA::set_rx_buffer(uint32_t size)
{
    if (size == 0)
    {
        return false;
    }
    if (this->rx_len == 0)
    {
        this->rx_buf = (char *)malloc(size);
    }
    else
    {
        this->rx_buf = (char *)realloc(this->rx_buf, size);
    }
    if (this->rx_buf)
    {
        this->rx_end = 0;
        this->rx_len = size;
    }
    return this->rx_buf != NULL;
}

bool SSCMA::set_tx_buffer(uint32_t size)
{
   if (size == 0)
    {
        return false;
    }
    if (this->tx_len == 0)
    {
        this->tx_buf = (char *)malloc(size);
    }
    else
    {
        this->tx_buf = (char *)realloc(this->tx_buf, size);
    }
    if (this->tx_buf)
    {
        this->tx_len = size;
    }
    return this->tx_buf != nullptr;
}