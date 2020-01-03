#include "imu_rev.h"
#include <string.h>

static char rev_buf[64];
static imu_data data;

enum input_status
{
    STATUS_IDLE,
    STATUS_SOF,
    STATUS_LEN,
    STATUS_DATA,
    STATUS_FCS,
};

void imu_rev_init(void)
{
    
}

void imu_get_data(imu_data *imu)
{
    memcpy(imu, &data, sizeof(imu_data));
}

int imu_rev_phase_data(imu_data* data)
{
    data->accl[0] = (rev_buf[0]<<8) + rev_buf[1];
    data->accl[1] = (rev_buf[2]<<8) + rev_buf[3];
    data->accl[2] = (rev_buf[4]<<8) + rev_buf[5];

    data->gyro[0] = (rev_buf[6]<<8) + rev_buf[7];
    data->gyro[1] = (rev_buf[8]<<8) + rev_buf[9];
    data->gyro[2] = (rev_buf[10]<<8) + rev_buf[11];
    
    data->mag[0] = (rev_buf[12]<<8) + rev_buf[13];
    data->mag[1] = (rev_buf[14]<<8) + rev_buf[15];
    data->mag[2] = (rev_buf[16]<<8) + rev_buf[17];
    
    data->roll = (rev_buf[18]<<8) + rev_buf[19];
    data->pitch = (rev_buf[20]<<8) + rev_buf[21];
    data->yaw = (rev_buf[22]<<8) + rev_buf[23];
    
    data->presure = (rev_buf[27]<<24) + (rev_buf[26]<<16) + (rev_buf[25]<<8) + (rev_buf[24]<<0);
    
    return 0;
}

void imu_rev_process(char ch)
{
    static int len = 0;
    static int i;
    uint8_t fcs_frame;
    uint8_t fcs = 0;
    
    static enum input_status status = STATUS_IDLE;
    
    /* running status machine */
    switch(status)
    {
        case STATUS_IDLE:
            if((uint8_t)ch == 0x88)
            {
                status = STATUS_SOF;
            }
            break;
        case STATUS_SOF:
            if((uint8_t)ch == 0xAF)
            {
                status = STATUS_LEN;
            }
            break;
        case STATUS_LEN:
            len = ch;
            if(len > sizeof(rev_buf))
            {
                status = STATUS_IDLE;
            }
            else
            {
                status = STATUS_DATA;
            }
            i = 0;
            break;
        case STATUS_DATA:
            rev_buf[i++] = ch;
        
            if(i == len)
            {
                status = STATUS_FCS;
            }
            break;
        case STATUS_FCS:
            fcs_frame = ch;
            fcs = 0x88;
            fcs += 0xAF;
            fcs += len;
            for(i=0; i<27; i++)
            {
                fcs += rev_buf[i];
            }
            
            /* checksum is correct */
            if(fcs == fcs_frame)
            {
                imu_rev_phase_data(&data);
            }
            status = STATUS_IDLE;
            break;
        default:
            break;
    }
}


