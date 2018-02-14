
#define FILE_ID         (1929976897)
#define FILE_VERSION    (60637938)
#define FILE_SIZE       (153)
#define NUM_SEGS        (2)

uint8_t MSG_INIT[] = {0x00,0x22,0x19,0x0E,0x09,0x41,0x1C,0x09,0x73,
                                 0xF2,0x42,0x9D,0x03,0x99,0x00,0x00,0x00};

uint8_t MSG_SEG1[] = {0x03,0xE7,0x14,0xFA,0xCF,0x41,0x1C,0x09,0x73,0xF2,0x42,0x9D,0x03,0x99,0x00,0x00,
                            0x00,0x00,0x00,0x02,0x00,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x31,
                            0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
                            0x38,0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x31,0x32,0x33,
                            0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x0A,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
                            0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x31,0x32,0x33,0x34,
                            0x35,0x36,0x37,0x38,0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,
                            0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x0A,0x31,0x32,0x33,0x34,0x35};

uint8_t MSG_SEG2[] = {0x03,0xB0,0xF2,0x54,0x70,0x41,0x1C,0x09,0x73,0xF2,0x42,0x9D,0x03,0x99,0x00,0x00,
                            0x00,0x01,0x00,0x02,0x00,0x36,0x37,0x38,0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36,
                            0x37,0x38,0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x31,0x32,
                            0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
                            0x39,0x30,0x0A};


uint8_t MSG_CANCEL[] = {0x01,0x22,0x19,0x0e,0x09,0x41,0x1C,0x09,0x73,
                                   0xF2,0x42,0x9D,0x03,0x99,0x00,0x00,0x00};


uint8_t MSG_APPLY[] = {0x02,0x22,0x19,0x0E,0x09,0x41,0x1C,0x09,0x73,
                                  0xF2,0x42,0x9D,0x03,0x99,0x00,0x00,0x00};


uint8_t MSG_ACK_INIT[] = {0x04,0xDF,0xC9,0xAA,0x38,0x00,0x41,0x1C,
                                          0x09,0x73,0xF2,0x42,0x9D,0x03};

uint8_t MSG_NACK_INIT_FILE_INFO_ZERO[] = {0x04,0xe5,0x18,0x46,0xaf,0xff,0x00,0x00,
                                          0x00,0x00,0x00,0x00,0x00,0x00};

uint8_t MSG_NACK_INIT[] = {0x04,0x77,0xc2,0x6a,0xff,0xff,0x42,0x1c,
                                          0x09,0x73,0xf2,0x42,0x9d,0x03};
    
uint8_t MSG_ACK_SEG[] = {0x05,0xDF,0xC9,0xAA,0x38,0x00,0x41,0x1C,
                                         0x09,0x73,0xF2,0x42,0x9D,0x03};

uint8_t MSG_NAK_SEG[] = {0x05,0xbb,0x92,0x8d,0xfe,0xfd,0x41,0x1c,
                               0x09,0x73,0xf2,0x42,0x9d,0x03,0x01,0x00};

uint8_t MSG_ACK_APPLY[] = {0x06,0xDF,0xC9,0xAA,0x38,0x00,0x41,0x1C,
                                           0x09,0x73,0xF2,0x42,0x9D,0x03};