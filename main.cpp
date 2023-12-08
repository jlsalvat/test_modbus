#include "mbed.h"
#define FC03 3
#define ADDRESS 26
BufferedSerial serial(D5, D4, 19200);
DigitalOut txen(D12);
DigitalIn In1(D6);
DigitalIn In2(D7);

char cInputBuffer[128];
char cResponse[128] = {11};

void crc(int start, int cnt)
{
  int i, j;
  unsigned int temp, temp2, flag;
  temp = 0xFFFF;
  for (i = start; i < cnt; i++)
  {
    temp = temp ^ cResponse[i];

    for (j = 1; j <= 8; j++)
    {
      flag = temp & 0x0001;
      temp = temp >> 1;
      if (flag)
        temp = temp ^ 0xA001;
    }
  }
  temp2 = temp >> 8;
  temp = (temp << 8) | temp2;
  cResponse[cnt] = temp / 256;
  cResponse[cnt + 1] = temp % 256;
}

void reply_ok()
{
  static int cpt = 1;
  cResponse[1] = FC03;
  cResponse[2] = 2; // nombre d’octets lus
  cResponse[3] = 0;
  cResponse[4] = cpt++; //+ In2 << 1 + In1;
  crc(0, 5);
  serial.write(cResponse, 7);
  for (int i = 0; i < 7; i++)
  {
    //      serial.putc(cResponse[i]);
    printf("%x ", cResponse[i]);
  }
}

void reply_ko()
{
  cResponse[1] = 0x80 + cInputBuffer[1]; // erreur
  cResponse[2] = 1;                      // le code erreur peut être
  crc(0, 3);
  serial.write(cResponse, 5);
  for (int i = 0; i < 5; i++)
  {
    //      serial.putc(cResponse[i]);
    printf("%x ", cResponse[i]);
  }
}

int main()
{
  int error, nAddress;

  serial.set_format(8, BufferedSerial::Even, 1);
  while (1)
  {
    error = 1;
    txen = 0; // mode réception
    /*for (int i = 0; i < 8; i++) {
      cInputBuffer[i] = serial.getc(); // lecture de la trame, octet par octet
    }*/
    char tmp;
    for (int i = 0; i < 8; i++)
    {
      serial.read(&tmp, 1);
      cInputBuffer[i] = tmp;
      printf("%x ", cInputBuffer[i]);
    }
    printf("\n\r");
    if (cInputBuffer[0] == cResponse[0])
    {
      if (cInputBuffer[1] == FC03)
      {
        nAddress = cInputBuffer[2] * 256 + cInputBuffer[3];
        if (nAddress == ADDRESS)
          error = 0;
      }
      txen = 1; // mode émission
      printf("addresse=%d\n\r", nAddress);
      if (error == 0)
        reply_ok();
      else
        reply_ko();
      printf("\n\r");
    }
  }
  
}
