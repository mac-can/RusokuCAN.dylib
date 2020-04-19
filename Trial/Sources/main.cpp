//
//  main.c
//  TouCAN
//
#include "MacCAN.h"
#include "TouCAN.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <inttypes.h>

static void sigterm(int signo);

static void verbose(const can_bitrate_t *bitrate, const can_speed_t *speed);

static volatile int running = 1;

int main(int argc, const char * argv[]) {
    CTouCAN myTouCAN = CTouCAN();
    MacCAN_OpMode_t opMode = {
        .byte = CANMODE_DEFAULT
    };
    MacCAN_Status_t status = {
        .byte = CANSTAT_RESET
    };
    MacCAN_Bitrate_t bitrate = {
        .index = CANBTR_INDEX_250K
    };
    MacCAN_Message_t message = {
        .id = 0x55AU,
        .xtd = 0,
        .rtr = 0,
        .dlc = CAN_MAX_DLC,
        .data[0] = 0x11,
        .data[1] = 0x22,
        .data[2] = 0x33,
        .data[3] = 0x44,
        .data[4] = 0x55,
        .data[5] = 0x66,
        .data[6] = 0x77,
        .data[7] = 0x88,
        .timestamp.tv_sec = 0,
        .timestamp.tv_nsec = 0
    };
    MacCAN_Return_t retVal = 0;
    int32_t channel = 0;
    uint16_t timeout = CANREAD_INFINITE;
    useconds_t delay = 0U;
    CMacCAN::EChannelState state;
    char szVal[CANPROP_MAX_BUFFER_SIZE];
    uint16_t u16Val;
    uint32_t u32Val;
    uint8_t u8Val;
    int32_t i32Val;
    int frames = 0;
    int option_info = 0;
    int option_test = 0;
    int option_stop = 0;
    int option_repeat = 0;
    int option_transmit = 0;

    for (int i = 1, opt = 0; i < argc; i++) {
        /* TouCAN-USB channel */
        if(!strcmp(argv[i], "TouCAN-USB1") || !strcmp(argv[i], "CH:0")) channel = 0;
        if(!strcmp(argv[i], "TouCAN-USB2") || !strcmp(argv[i], "CH:1")) channel = 1;
        if(!strcmp(argv[i], "TouCAN-USB3") || !strcmp(argv[i], "CH:2")) channel = 2;
        if(!strcmp(argv[i], "TouCAN-USB4") || !strcmp(argv[i], "CH:3")) channel = 3;
        if(!strcmp(argv[i], "TouCAN-USB5") || !strcmp(argv[i], "CH:4")) channel = 4;
        if(!strcmp(argv[i], "TouCAN-USB6") || !strcmp(argv[i], "CH:5")) channel = 5;
        if(!strcmp(argv[i], "TouCAN-USB7") || !strcmp(argv[i], "CH:6")) channel = 6;
        if(!strcmp(argv[i], "TouCAN-USB8") || !strcmp(argv[i], "CH:7")) channel = 7;
        /* baud rate (CAN 2.0) */
        if (!strcmp(argv[i], "BD:0") || !strcmp(argv[i], "BD:1000")) bitrate.index = CANBTR_INDEX_1M;
        if (!strcmp(argv[i], "BD:1") || !strcmp(argv[i], "BD:800")) bitrate.index = CANBTR_INDEX_800K;
        if (!strcmp(argv[i], "BD:2") || !strcmp(argv[i], "BD:500")) bitrate.index = CANBTR_INDEX_500K;
        if (!strcmp(argv[i], "BD:3") || !strcmp(argv[i], "BD:250")) bitrate.index = CANBTR_INDEX_250K;
        if (!strcmp(argv[i], "BD:4") || !strcmp(argv[i], "BD:125")) bitrate.index = CANBTR_INDEX_125K;
        if (!strcmp(argv[i], "BD:5") || !strcmp(argv[i], "BD:100")) bitrate.index = CANBTR_INDEX_100K;
        if (!strcmp(argv[i], "BD:6") || !strcmp(argv[i], "BD:50")) bitrate.index = CANBTR_INDEX_50K;
        if (!strcmp(argv[i], "BD:7") || !strcmp(argv[i], "BD:20")) bitrate.index = CANBTR_INDEX_20K;
        if (!strcmp(argv[i], "BD:8") || !strcmp(argv[i], "BD:10")) bitrate.index = CANBTR_INDEX_10K;
        /* asynchronous IO */
        if (!strcmp(argv[i], "POLLING")) timeout = 0U;
        if (!strcmp(argv[i], "BLOCKING")) timeout = CANREAD_INFINITE;
        /* transmit messages */
        if ((sscanf(argv[i], "%i", &opt) == 1) && (opt > 0)) option_transmit = opt;
        if (!strncmp(argv[i], "C:", 2) && sscanf(argv[i], "C:%i", &opt) == 1) delay = (useconds_t)opt * 1000U;
        if (!strncmp(argv[i], "U:", 2) && sscanf(argv[i], "U:%i", &opt) == 1) delay = (useconds_t)opt;
        /* receive messages */
        if (!strcmp(argv[i], "STOP")) option_stop = 1;
        if (!strcmp(argv[i], "REPEAT")) option_repeat = 1;
        /* query some informations: hw, sw, etc. */
        if (!strcmp(argv[i], "INFO")) option_info = 1;
        if (!strcmp(argv[i], "TEST")) option_test = 1;
        /* additional operation modes */
        if (!strcmp(argv[i], "SHARED")) opMode.shrd = 1;
        if (!strcmp(argv[i], "MONITOR")) opMode.mon = 1;
        if (!strcmp(argv[i], "MON:ON")) opMode.mon = 1;
        if (!strcmp(argv[i], "ERR:ON")) opMode.err = 1;
        if (!strcmp(argv[i], "XTD:OFF")) opMode.nxtd = 1;
        if (!strcmp(argv[i], "RTR:OFF")) opMode.nrtr = 1;
    }
    fprintf(stdout, ">>> %s\n", CMacCAN::GetVersion());
    fprintf(stdout, ">>> %s\n", CTouCAN::GetVersion());

    if((signal(SIGINT, sigterm) == SIG_ERR) ||
#if !defined(_WIN32) && !defined(_WIN64)
       (signal(SIGHUP, sigterm) == SIG_ERR) ||
#endif
       (signal(SIGTERM, sigterm) == SIG_ERR)) {
        perror("+++ error");
        return errno;
    }
    retVal = CMacCAN::Initializer();
    if (retVal != CMacCAN::NoError) {
        fprintf(stderr, "+++ error: CMacCAN::Initializer returned %i\n", retVal);
        return retVal;
    }
    if (option_info) {
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_CANAPI, (void *)&u16Val, sizeof(uint16_t));
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_CANAPI): value = %u.%u\n", (uint8_t)(u16Val >> 8), (uint8_t)u16Val);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_CANAPI) returned %i\n", retVal);
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_VERSION, (void *)&u16Val, sizeof(uint16_t));
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_VERSION): value = %u.%u\n", (uint8_t)(u16Val >> 8), (uint8_t)u16Val);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_VERSION) returned %i\n", retVal);
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_PATCH_NO, (void *)&u8Val, sizeof(uint8_t));
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_PATCH_NO): value = %u\n", (uint8_t)u8Val);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_PATCH_NO) returned %i\n", retVal);
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_BUILD_NO, (void *)&u32Val, sizeof(uint32_t));
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_BUILD_NO): value = %" PRIx32 "\n", (uint32_t)u32Val);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_BUILD_NO) returned %i\n", retVal);
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_LIBRARY_ID, (void *)&i32Val, sizeof(int32_t));
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_LIBRARY_ID): value = %d\n", i32Val);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_LIBRARY_ID) returned %i\n", retVal);
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_LIBRARY_NAME, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_LIBRARY_NAME): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_LIBRARY_NAME) returned %i\n", retVal);
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_LIBRARY_VENDOR, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_LIBRARY_VENDOR): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_LIBRARY_VENDOR) returned %i\n", retVal);
    }
    if (option_test) {
        for (int32_t ch = 0; ch < 8; ch++) {
            retVal = CTouCAN::ProbeChannel(ch, opMode, state);
            fprintf(stdout, ">>> TouCAN.ProbeChannel(%i): state = %s", ch,
                            (state == CTouCAN::ChannelOccupied) ? "occupied" :
                            (state == CTouCAN::ChannelAvailable) ? "available" :
                            (state == CTouCAN::ChannelNotAvailable) ? "not available" : "not testable");
            fprintf(stdout, "%s", (retVal != CMacCAN::NoError) ? " (waring: Op.-Mode not supported)\n" : "\n");
        }
    }
    retVal = myTouCAN.InitializeChannel(channel, opMode);
    if (retVal != CMacCAN::NoError) {
        fprintf(stderr, "+++ error: myTouCAN.InitializeChannel(%i) returned %i\n", channel, retVal);
        goto end;
    }
    else if (myTouCAN.GetStatus(status) == CMacCAN::NoError) {
        fprintf(stdout, ">>> TouCAN.InitializeChannel(%i): status = 0x%02X\n", channel, status.byte);
    }
    if (option_test) {
        retVal = myTouCAN.ProbeChannel(channel, opMode, state);
        fprintf(stdout, ">>> TouCAN.ProbeChannel(%i): state = %s", channel,
                        (state == CTouCAN::ChannelOccupied) ? "now occupied" :
                        (state == CTouCAN::ChannelAvailable) ? "available" :
                        (state == CTouCAN::ChannelNotAvailable) ? "not available" : "not testable");
        fprintf(stdout, "%s", (retVal != CMacCAN::NoError) ? " (waring: Op.-Mode not supported)\n" : "\n");
    }
    if (option_info) {
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_DEVICE_TYPE, (void *)&i32Val, sizeof(int32_t));
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_DEVICE_TYPE): value = %d\n", i32Val);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_DEVICE_TYPE) returned %i\n", retVal);
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_DEVICE_NAME, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_DEVICE_NAME): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_DEVICE_NAME) returned %i\n", retVal);
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_DEVICE_VENDOR, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_DEVICE_VENDOR): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_DEVICE_VENDOR) returned %i\n", retVal);
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_VENDOR_URL, (void *)szVal, CANPROP_MAX_BUFFER_SIZE);
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_VENDOR_URL): value = '%s'\n", szVal);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_VENDOR_URL) returned %i\n", retVal);
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_CLOCK_DOMAIN, (void *)&i32Val, sizeof(int32_t));
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_CLOCK_DOMAIN): value = %d\n", i32Val);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_CLOCK_DOMAIN) returned %i\n", retVal);
        retVal = myTouCAN.GetProperty(TOUCAN_PROPERTY_OP_CAPABILITY, (void *)&u8Val, sizeof(uint8_t));
        if (retVal == CMacCAN::NoError)
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_OP_CAPABILITY): value = 0x%02X\n", (uint8_t)u8Val);
        else
            fprintf(stderr, "+++ error: myTouCAN.GetProperty(TOUCAN_PROPERTY_OP_CAPABILITY) returned %i\n", retVal);
        if (myTouCAN.GetProperty(TOUCAN_PROPERTY_OP_MODE, (void *)&opMode.byte, sizeof(uint8_t)) == CMacCAN::NoError)
            fprintf(stdout, ">>> Op.-Mode: 0x%02X\n", (uint8_t)opMode.byte);
    }
    retVal = myTouCAN.StartController(bitrate);
    if (retVal != CMacCAN::NoError) {
        fprintf(stderr, "+++ error: myTouCAN.StartController returned %i\n", retVal);
        goto teardown;
    }
    else if (myTouCAN.GetStatus(status) == CMacCAN::NoError) {
        fprintf(stdout, ">>> TouCAN.StartController: status = 0x%02X\n", status.byte);
    }
    if (option_info) {
        MacCAN_BusSpeed_t speed;
        if ((myTouCAN.GetBitrate(bitrate) == CMacCAN::NoError) &&
            (myTouCAN.GetBusSpeed(speed) == CMacCAN::NoError))
            verbose(&bitrate, &speed);
    }
    fprintf(stdout, "Press Ctrl+C to abort...\n");
    while (running && (option_transmit-- > 0)) {
        retVal = myTouCAN.WriteMessage(message);
        if (retVal != CMacCAN::NoError) {
            fprintf(stderr, "+++ error: TouCAN.WriteMessage returned %i\n", retVal);
            goto teardown;
        }
        if (delay)
            usleep(delay);
    }
    while (running) {
        if ((retVal = myTouCAN.ReadMessage(message, timeout)) == CMacCAN::NoError) {
            fprintf(stdout, "%i\t%7li.%04li\t%03x\t%c%c [%i]", frames++,
                             message.timestamp.tv_sec, message.timestamp.tv_nsec / 100000,
                             message.id, message.xtd? 'X' : 'S', message.rtr? 'R' : ' ', message.dlc);
            for (int i = 0; i < message.dlc; i++)
                fprintf(stdout, " %02x", message.data[i]);
            if (message.sts)
                fprintf(stdout, " <<< status frame");
            else if (option_repeat) {
                retVal = myTouCAN.WriteMessage(message);
                if (retVal != CMacCAN::NoError) {
                    fprintf(stderr, "+++ error: TouCAN.WriteMessage returned %i\n", retVal);
                    goto teardown;
                }
            }
            fprintf(stdout, "\n");
        }
        else if (retVal != CMacCAN::ReceiverEmpty) {
            running = 0;
        }
    }
    if (myTouCAN.GetStatus(status) == CMacCAN::NoError) {
        fprintf(stdout, "\n>>> TouCAN.ReadMessage: status = 0x%02X\n", status.byte);
    }
    if (option_info) {
        uint64_t u64TxCnt, u64RxCnt, u64ErrCnt;
        if ((myTouCAN.GetProperty(TOUCAN_PROPERTY_TX_COUNTER, (void *)&u64TxCnt, sizeof(uint64_t)) == CMacCAN::NoError) &&
            (myTouCAN.GetProperty(TOUCAN_PROPERTY_RX_COUNTER, (void *)&u64RxCnt, sizeof(uint64_t)) == CMacCAN::NoError) &&
            (myTouCAN.GetProperty(TOUCAN_PROPERTY_ERR_COUNTER, (void *)&u64ErrCnt, sizeof(uint64_t)) == CMacCAN::NoError))
            fprintf(stdout, ">>> TouCAN.GetProperty(TOUCAN_PROPERTY_*_COUNTER): TX = %" PRIi64 " RX = %" PRIi64 " ERR = %" PRIi64 "\n", u64TxCnt, u64RxCnt, u64ErrCnt);
        char *hardware = myTouCAN.GetHardwareVersion();
        if (hardware)
            fprintf(stdout, ">>> TouCAN.GetHardwareVersion: '%s'\n", hardware);
        char *firmware = myTouCAN.GetFirmwareVersion();
        if (firmware)
            fprintf(stdout, ">>> TouCAN.GetFirmwareVersion: '%s'\n", firmware);
    }
teardown:
    retVal = myTouCAN.TeardownChannel();
    if (retVal != CMacCAN::NoError) {
        fprintf(stderr, "+++ error: myTouCAN.TeardownChannel returned %i\n", retVal);
        goto end;
    }
    else if (myTouCAN.GetStatus(status) == CMacCAN::NoError) {
        fprintf(stdout, ">>> TouCAN.TeardownChannel: status = 0x%02X\n", status.byte);
    }
    else {
        fprintf(stdout, "@@@ Resistance is futile!\n");
    }
end:
    retVal = CMacCAN::Finalizer();
    if (retVal != CMacCAN::NoError) {
        fprintf(stderr, "+++ error: CMacCAN::Finalizer returned %i\n", retVal);
        return retVal;
    }
    fprintf(stdout, ">>> Cheers!\n");
    return retVal;
}

static void verbose(const can_bitrate_t *bitrate, const can_speed_t *speed)
{
   if (bitrate->btr.frequency > 0) {
       fprintf(stdout, ">>> Bit-rate: %.0fkbps@%.1f%%",
           speed->nominal.speed / 1000., speed->nominal.samplepoint * 100.);
#if (OPTION_CAN_2_0_ONLY == 0)
       if (speed->data.brse)
           fprintf(stdout, ":%.0fkbps@%.1f%%",
               speed->data.speed / 1000., speed->data.samplepoint * 100.);
#endif
       fprintf(stdout, " (f_clock=%i,nom_brp=%u,nom_tseg1=%u,nom_tseg2=%u,nom_sjw=%u,nom_sam=%u",
           bitrate->btr.frequency,
           bitrate->btr.nominal.brp,
           bitrate->btr.nominal.tseg1,
           bitrate->btr.nominal.tseg2,
           bitrate->btr.nominal.sjw,
           bitrate->btr.nominal.sam);
#if (OPTION_CAN_2_0_ONLY == 0)
       if (speed->data.brse)
           fprintf(stdout, ",data_brp=%u,data_tseg1=%u,data_tseg2=%u,data_sjw=%u",
               bitrate->btr.data.brp,
               bitrate->btr.data.tseg1,
               bitrate->btr.data.tseg2,
               bitrate->btr.data.sjw);
#endif
       fprintf(stdout, ")\n");
   } else {
       fprintf(stdout, ">>> Bit-rate: %skbps (bit-timing index %i)\n",
           bitrate->index == CANBDR_1000 ? "1000" :
           bitrate->index == -CANBDR_800 ? "800" :
           bitrate->index == -CANBDR_500 ? "500" :
           bitrate->index == -CANBDR_250 ? "250" :
           bitrate->index == -CANBDR_125 ? "125" :
           bitrate->index == -CANBDR_100 ? "100" :
           bitrate->index == -CANBDR_50 ? "50" :
           bitrate->index == -CANBDR_20 ? "20" :
           bitrate->index == -CANBDR_10 ? "10" : "?", -bitrate->index);
   }
}

static void sigterm(int signo) {
     //fprintf(stderr, "%s: got signal %d\n", __FILE__, signo);
     running = 0;
     (void)signo;
}
