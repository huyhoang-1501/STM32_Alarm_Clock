/*
 * main.c
 *
 *  Created on: Mar 1, 2026
 *      Author: DoubleH
 */
#include "main.h"
#include "i2c-lcd.h"
#include <stdio.h>
#include <string.h>

/*
  Notes:
  - Buttons: PA5 = UP (len), PA6 = DOWN (xuong), PA4 = MENU.
  - Buzzer on PA8 (coi).
*/

/* I2C handle used by i2c-lcd.c */
I2C_HandleTypeDef hi2c1;

#define DS3231_ADDRESS 0xD0

typedef struct {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hour;
  uint8_t dayofweek;
  uint8_t dayofmonth;
  uint8_t month;
  uint8_t year;
} TIME;

TIME time;
char buffer[32];

/* Forward prototypes for MCU init functions already in your project */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

/* DS3231 helpers (same as before) */
static uint8_t decToBcd(int val) { return (uint8_t)(((val/10)*16) + (val%10)); }
static int bcdToDec(uint8_t val)   { return (int)(((val/16)*10) + (val%16)); }

static void Set_Time (uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month_, uint8_t year)
{
  uint8_t set_time[7];
  set_time[0] = decToBcd(sec);
  set_time[1] = decToBcd(min);
  set_time[2] = decToBcd(hour);
  set_time[3] = decToBcd(dow);
  set_time[4] = decToBcd(dom);
  set_time[5] = decToBcd(month_);
  set_time[6] = decToBcd(year);
  HAL_I2C_Mem_Write(&hi2c1, DS3231_ADDRESS, 0x00, 1, set_time, 7, 1000);
}

static void Get_Time (void)
{
  uint8_t get_time[7] = {0};
  if (HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x00, 1, get_time, 7, 1000) == HAL_OK) {
    time.seconds = bcdToDec(get_time[0]);
    time.minutes = bcdToDec(get_time[1]);
    time.hour = bcdToDec(get_time[2]);
    time.dayofweek = bcdToDec(get_time[3]);
    time.dayofmonth = bcdToDec(get_time[4]);
    time.month = bcdToDec(get_time[5]);
    time.year = bcdToDec(get_time[6]);
  }
}

static void lcd_cursor_on(void)  { lcd_send_cmd(0x0E); } /* Display ON, Cursor ON, Blink OFF */
static void lcd_cursor_off(void) { lcd_send_cmd(0x0C); } /* Display ON, Cursor OFF, Blink OFF */

/* Pin mapping (same as earlier ports) */
#define BTN_UP_PORT    GPIOA
#define BTN_UP_PIN     GPIO_PIN_5  /* len */
#define BTN_DOWN_PORT  GPIOA
#define BTN_DOWN_PIN   GPIO_PIN_6  /* xuong */
#define BTN_MENU_PORT  GPIOA
#define BTN_MENU_PIN   GPIO_PIN_4  /* menu */
#define BUZZER_PORT    GPIOA
#define BUZZER_PIN     GPIO_PIN_8  /* coi */

int len = 6; int gtlen;
int xuong = 5; int gtxuong;
int menu = 7; int gtmenu;
int macdinh = 1;
int coi = 8;

int contro = 0; int contro_bt = 6; int hang = 0;

int congtru_tong = 0; int congtru_menubaothuc = 0;
int demtong = 0;  int dembaothuc = 0;

int ngay = 1; int thang = 1; int namng = 0; int namtr = 0; int namch = 0; int namdv = 0; int namtong = 0; //SETUP DATE
int gio = 0; int phut = 0; int giay = 0; //SETUP TIME

int gio1 = 0; int phut1 = 0; int ngay1 = 1; int thang1 = 1; //BÁO THỨC 1
int gio2 = 0; int phut2 = 0; int ngay2 = 1; int thang2 = 1; //BÁO THỨC 2
int gio3 = 0; int phut3 = 0; int ngay3 = 1; int thang3 = 1; //BÁO THỨC 3
int gio4 = 0; int phut4 = 0; int ngay4 = 1; int thang4 = 1; //BÁO THỨC 4
int gio5 = 0; int phut5 = 0; int ngay5 = 1; int thang5 = 1; //BÁO THỨC 5

/* Main screen */
static void manhinh(void)
{
  lcd_put_cur(0,0); lcd_send_string("DATE: ");
  lcd_put_cur(1,0); lcd_send_string("TIME: ");
  lcd_put_cur(0,6);
  snprintf(buffer, sizeof(buffer), "%02d-%02d-20%02d", time.dayofmonth, time.month, time.year);
  lcd_send_string(buffer);
  lcd_put_cur(1,6);
  snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", time.hour, time.minutes, time.seconds);
  lcd_send_string(buffer);
}

/* Top menu */
static void menu_tong(void)
{
  if (congtru_tong == 0)
  {
    lcd_clear();
    lcd_put_cur(0,0); lcd_send_string(">BACK");
    lcd_put_cur(1,0); lcd_send_string(" DATE");
  }
  else if (congtru_tong == 1)
  {
    lcd_clear();
    lcd_put_cur(0,0); lcd_send_string(" BACK");
    lcd_put_cur(1,0); lcd_send_string(">DATE");
  }
  else if (congtru_tong == 2)
  {
    lcd_clear();
    lcd_put_cur(0,0); lcd_send_string(">TIME");
    lcd_put_cur(1,0); lcd_send_string(" ALARM");
  }
  else if (congtru_tong == 3)
  {
    lcd_clear();
    lcd_put_cur(0,0); lcd_send_string(" TIME");
    lcd_put_cur(1,0); lcd_send_string(">ALARM");
  }
}

/* Choose top menu item */
static void chonmenu_tong(void)
{
  switch (congtru_tong)
  {
    case 0:
      /* BACK */
      break;
    case 1:
      lcd_clear();
      lcd_put_cur(0,0); lcd_send_string("SETUP DATE");
      lcd_put_cur(1,12); lcd_send_string("BACK");
      break;
    case 2:
      lcd_clear();
      lcd_put_cur(0,0); lcd_send_string("SETUP TIME");
      lcd_put_cur(1,12); lcd_send_string("BACK");
      break;
    case 3:
      /* ALARM */
      break;
  }
}

/* Alarm menu */
static void menu_baothuc(void)
{
  if (congtru_menubaothuc == 0)
  {
    lcd_clear();
    lcd_put_cur(0,0); lcd_send_string(">BACK");
    lcd_put_cur(1,0); lcd_send_string(" ALARM 1");
  }
  else if (congtru_menubaothuc == 1)
  {
    lcd_clear();
    lcd_put_cur(0,0); lcd_send_string(" BACK");
    lcd_put_cur(1,0); lcd_send_string(">ALARM 1");
  }
  else if (congtru_menubaothuc == 2)
  {
    lcd_clear();
    lcd_put_cur(0,0); lcd_send_string(">ALARM 2");
    lcd_put_cur(1,0); lcd_send_string(" ALARM 3");
  }
  else if (congtru_menubaothuc == 3)
  {
    lcd_clear();
    lcd_put_cur(0,0); lcd_send_string(" ALARM 2");
    lcd_put_cur(1,0); lcd_send_string(">ALARM 3");
  }
  else if (congtru_menubaothuc == 4)
  {
    lcd_clear();
    lcd_put_cur(0,0); lcd_send_string(">ALARM 4");
    lcd_put_cur(1,0); lcd_send_string(" ALARM 5");
  }
  else if (congtru_menubaothuc == 5)
  {
    lcd_clear();
    lcd_put_cur(0,0); lcd_send_string(" ALARM 4");
    lcd_put_cur(1,0); lcd_send_string(">ALARM 5");
  }
}

/* Choose alarm submenu */
static void chonmenu_baothuc(void)
{
  switch (congtru_menubaothuc)
  {
    case 0:
      /* BACK */
      break;
    case 1:
      lcd_clear();
      lcd_put_cur(0,0); lcd_send_string("DATE: ");
      lcd_put_cur(1,0); lcd_send_string("TIME: ");
      lcd_put_cur(1,12); lcd_send_string("BACK");
      break;
    default:
      lcd_clear();
      lcd_put_cur(0,0); lcd_send_string("DATE: ");
      lcd_put_cur(1,0); lcd_send_string("TIME: ");
      lcd_put_cur(1,12); lcd_send_string("BACK");
      break;
  }
}

/* Alarm notify */
static void tb_baothuc(void)
{
  lcd_clear();
  lcd_put_cur(0,0); lcd_send_string("ALARM TRIGGERED!");
  lcd_put_cur(1,2); lcd_send_string(" CHECK TIME ");
}

/* Utility edit helpers: use pointers so C calls pass &var */
static void tru_ngay_gio(int *pngay, int *pgio)
{
  if (hang == 0){
    (*pngay)--;
    if (*pngay < 1) *pngay = 31;
  } else {
    (*pgio)--;
    if (*pgio < 0) *pgio = 23;
  }
}
static void tru_thang_phut(int *pthang, int *pphut)
{
  if (hang == 0){
    (*pthang)--;
    if (*pthang < 1) *pthang = 12;
  } else {
    (*pphut)--;
    if (*pphut < 0) *pphut = 59;
  }
}
static void cong_ngay_gio(int *pngay, int *pgio)
{
  if (hang == 0){
    (*pngay)++;
    if (*pngay > 31) *pngay = 1;
  } else {
    (*pgio)++;
    if (*pgio > 23) *pgio = 0;
  }
}
static void cong_thang_phut(int *pthang, int *pphut)
{
  if (hang == 0){
    (*pthang)++;
    if (*pthang > 12) *pthang = 1;
  } else {
    (*pphut)++;
    if (*pphut > 59) *pphut = 0;
  }
}

/* hien_thi_bt: display alarm edit */
static void hien_thi_bt(int *pngay, int *pthang, int *pgio, int *pphut)
{
  if (*pngay < 10){
    lcd_put_cur(0,6); lcd_send_string("0");
    lcd_put_cur(0,7); snprintf(buffer,3,"%d", *pngay); lcd_send_string(buffer);
  }
  else { lcd_put_cur(0,6); snprintf(buffer,3,"%d", *pngay); lcd_send_string(buffer); }

  lcd_put_cur(0,8); lcd_send_string("/");

  if (*pthang < 10){
    lcd_put_cur(0,9); lcd_send_string("0");
    lcd_put_cur(0,10); snprintf(buffer,3,"%d", *pthang); lcd_send_string(buffer);
  } else { lcd_put_cur(0,9); snprintf(buffer,3,"%d", *pthang); lcd_send_string(buffer); }

  if (*pgio < 10){
    lcd_put_cur(1,6); lcd_send_string("0");
    lcd_put_cur(1,7); snprintf(buffer,3,"%d", *pgio); lcd_send_string(buffer);
  } else { lcd_put_cur(1,6); snprintf(buffer,3,"%d", *pgio); lcd_send_string(buffer); }

  lcd_put_cur(1,8); lcd_send_string(":");

  if (*pphut < 10){
    lcd_put_cur(1,9); lcd_send_string("0");
    lcd_put_cur(1,10); snprintf(buffer,3,"%d", *pphut); lcd_send_string(buffer);
  } else { lcd_put_cur(1,9); snprintf(buffer,3,"%d", *pphut); lcd_send_string(buffer); }

  /* place cursor at contro_bt, hang and enable cursor */
  lcd_put_cur(hang, contro_bt);
  lcd_cursor_on();
  HAL_Delay(50);
}

/* buzzer helpers */
static void buzzer_on(void)  { HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET); }
static void buzzer_off(void) { HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET); }

/* -------------------------------------------------------------------
   New alarm handling helpers
   - alarm_stop_requested() returns true if user pressed any button to stop alarm.
   - handle_alarm_event() toggles buzzer and displays notification until
     button pressed or timeout.
   -------------------------------------------------------------------*/
static uint8_t alarm_stop_requested(void)
{
  /* Active-low buttons: pressed == GPIO_PIN_RESET */
  if (HAL_GPIO_ReadPin(BTN_MENU_PORT, BTN_MENU_PIN) == GPIO_PIN_RESET) return 1;
  if (HAL_GPIO_ReadPin(BTN_UP_PORT,   BTN_UP_PIN)   == GPIO_PIN_RESET) return 1;
  if (HAL_GPIO_ReadPin(BTN_DOWN_PORT, BTN_DOWN_PIN) == GPIO_PIN_RESET) return 1;
  return 0;
}

static void handle_alarm_event(void)
{
  const uint32_t blink_ms = 500;      /* buzzer toggle period (ms) */
  const uint32_t timeout_ms = 60000;  /* auto-stop after 60 seconds */
  uint32_t start = HAL_GetTick();
  uint32_t last_toggle = start;
  uint8_t buz_state = 0;

  /* Show alarm screen */
  tb_baothuc();

  /* Toggle buzzer until user stops or timeout */
  while (1) {
    uint32_t now = HAL_GetTick();

    if ((now - last_toggle) >= blink_ms) {
      last_toggle = now;
      buz_state = !buz_state;
      if (buz_state) buzzer_on();
      else buzzer_off();
    }

    /* if user pressed any button, stop alarm */
    if (alarm_stop_requested()) {
      buzzer_off();
      /* simple debounce/consume press */
      HAL_Delay(150);
      while (HAL_GPIO_ReadPin(BTN_MENU_PORT, BTN_MENU_PIN) == GPIO_PIN_RESET ||
             HAL_GPIO_ReadPin(BTN_UP_PORT,   BTN_UP_PIN)   == GPIO_PIN_RESET ||
             HAL_GPIO_ReadPin(BTN_DOWN_PORT, BTN_DOWN_PIN) == GPIO_PIN_RESET) {
        HAL_Delay(10);
      }
      dembaothuc = 0;
      break;
    }

    /* timeout */
    if ((now - start) >= timeout_ms) {
      buzzer_off();
      dembaothuc = 0;
      break;
    }

    HAL_Delay(10);
  }
}

/* MAIN */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();

  lcd_init();
  lcd_clear();

  /* initial read */
  Get_Time();

  /* main loop */
  for (;;)
  {
    /* read RTC/time (like t = rtc.getTime()) */
    Get_Time();

    /* read buttons (HIGH=1 idle because INPUT_PULLUP) */
    gtlen = (HAL_GPIO_ReadPin(BTN_UP_PORT, BTN_UP_PIN) == GPIO_PIN_SET) ? 1 : 0;
    gtxuong = (HAL_GPIO_ReadPin(BTN_DOWN_PORT, BTN_DOWN_PIN) == GPIO_PIN_SET) ? 1 : 0;
    gtmenu = (HAL_GPIO_ReadPin(BTN_MENU_PORT, BTN_MENU_PIN) == GPIO_PIN_SET) ? 1 : 0;

    /*UP button block */
    if (gtlen != macdinh)  /* NÚT LÊN */
    {
      if (gtlen == 0) /* Khi nhấn nút lên */
      {
        if (demtong == 1)   /* LÊN Ở MENU TỔNG  */
        {
          if (congtru_tong >= 3) congtru_tong = 0;
          else congtru_tong++;
          menu_tong();
        }

        else if (demtong == 2 && (congtru_tong == 1 || congtru_tong == 2))   /* + CON TRỎ ở SET DATE và TIME */
        {
          contro ++;
          if (contro > 15) contro = 0;
        }

        else if (demtong == 3) /* - ở ngày, tháng, năm, giờ, phút, giây và + con trỏ Báo Thức   */
        {
          if(congtru_tong == 1) /* - ở NGÀY, THÁNG, NĂM */
          {
            if ((contro == 0 || contro == 1)){ /* NGÀY */
              ngay --;
              if (ngay < 1) ngay = 31;
            }
            else if ((contro == 3 || contro == 4)){ /* THÁNG */
              thang --;
              if (thang < 1) thang = 12;
            }
            else if (contro == 6){ namng --; if (namng < 0) namng = 9; }
            else if (contro == 7){ namtr --; if (namtr < 0) namtr = 9; }
            else if (contro == 8){ namch --; if (namch < 0) namch = 9; }
            else if (contro == 9){ namdv --; if (namdv < 0) namdv = 9; }
          }
          else if (congtru_tong == 2) /* - ở GIỜ : PHÚT : GIÂY */
          {
            if ((contro == 0 || contro == 1)){ gio --; if (gio < 0) gio = 23; }
            else if ((contro == 3 || contro == 4)){ phut --; if (phut < 0) phut = 59; }
            else if ((contro == 6 || contro == 7)){ giay --; if (giay < 0) giay = 59; }
          }
          else if (congtru_tong == 3) /* + CON TRỎ ở BÁO THỨC */
          {
            contro_bt ++;
            if (hang == 0){
              if (contro_bt > 10) { contro_bt = 6; hang = 1; }
            } else {
              if (contro_bt > 15) { contro_bt = 6; hang = 0; }
            }
          }
        }

        else if (demtong == 2 && congtru_tong == 3) /* Lên ở MENU BÁO THỨC */
        {
          congtru_menubaothuc ++;
          if (congtru_menubaothuc > 5) congtru_menubaothuc = 0;
          menu_baothuc();
        }

        else if (demtong == 4 && congtru_tong == 3) /* - BÁO THỨC | NGÀY/THÁNG & GIỜ/PHÚT */
        {
          if(contro_bt == 6 || contro_bt == 7) /* Ngày or Giờ */
          {
            if (congtru_menubaothuc == 1) tru_ngay_gio(&ngay1, &gio1);
            else if (congtru_menubaothuc == 2) tru_ngay_gio(&ngay2, &gio2);
            else if (congtru_menubaothuc == 3) tru_ngay_gio(&ngay3, &gio3);
            else if (congtru_menubaothuc == 4) tru_ngay_gio(&ngay4, &gio4);
            else if (congtru_menubaothuc == 5) tru_ngay_gio(&ngay5, &gio5);
          }
          else if (contro_bt == 9 || contro_bt == 10) /* Tháng or phút */
          {
            if (congtru_menubaothuc == 1) tru_thang_phut(&thang1, &phut1);
            else if (congtru_menubaothuc == 2) tru_thang_phut(&thang2, &phut2);
            else if (congtru_menubaothuc == 3) tru_thang_phut(&thang3, &phut3);
            else if (congtru_menubaothuc == 4) tru_thang_phut(&thang4, &phut4);
            else if (congtru_menubaothuc == 5) tru_thang_phut(&thang5, &phut5);
          }
        }
        HAL_Delay(150);
      }
      macdinh = gtlen;
    }

    /* --- DOWN button block --- */
    if (gtxuong != macdinh) /* NÚT XUỐNG */
    {
      if (gtxuong == 0) /* Khi nhấn nút xuống */
      {
        if (demtong == 1)   /* XUỐNG Ở MENU TỔNG */
        {
          if (congtru_tong <= 0) congtru_tong = 3;
          else congtru_tong--;
          menu_tong();
        }

        else if (demtong == 2 && (congtru_tong == 1 || congtru_tong == 2)) /* - CON TRỎ ở SET DATE và TIME */
        {
          contro --;
          if (contro < 0) contro = 15;
        }

        else if (demtong == 3) /* + Ở Ngày, tháng, năm, giờ, phút, giây và - con trỏ Báo Thức */
        {
          if(congtru_tong == 1) /* + ở NGÀY, THÁNG, NĂM */
          {
            if (contro == 0 || contro == 1){ ngay ++; if (ngay > 31) ngay = 1; }
            else if (contro == 3 || contro == 4){ thang ++; if (thang > 12) thang = 1; }
            else if (contro == 6){ namng ++; if (namng > 9) namng = 0; }
            else if (contro == 7){ namtr ++; if (namtr > 9) namtr = 0; }
            else if (contro == 8){ namch ++; if (namch > 9) namch = 0; }
            else if (contro == 9){ namdv ++; if (namdv > 9) namdv = 0; }
          }
          else if (congtru_tong == 2) /* + ở GIỜ : PHÚT :GIÂY */
          {
            if (contro == 0 || contro == 1){ gio ++; if (gio > 23) gio = 0; }
            else if (contro == 3 || contro == 4){ phut ++; if (phut > 59) phut = 0; }
            else if (contro == 6 || contro == 7){ giay ++; if (giay > 59) giay = 0; }
          }
          else if (congtru_tong == 3) /* - CON TRỎ ở BÁO THỨC */
          {
            contro_bt --;
            if (hang == 0){
              if (contro_bt < 6) { contro_bt = 15; hang = 1; }
            } else {
              if (contro_bt < 6) { contro_bt = 10; hang = 0; }
            }
          }
        }

        else if (demtong == 2 && congtru_tong == 3) /* Xuống ở MENU BÁO THỨC */
        {
          congtru_menubaothuc --;
          if (congtru_menubaothuc < 0) congtru_menubaothuc = 5;
          menu_baothuc();
        }

        else if (demtong == 4 && congtru_tong == 3)  /* + BÁO THỨC | NGÀY/THÁNG & GIỜ/PHÚT */
        {
          if(contro_bt == 6 || contro_bt == 7) /* Ngày or Giờ */
          {
            if (congtru_menubaothuc == 1) cong_ngay_gio(&ngay1, &gio1);
            else if (congtru_menubaothuc == 2) cong_ngay_gio(&ngay2, &gio2);
            else if (congtru_menubaothuc == 3) cong_ngay_gio(&ngay3, &gio3);
            else if (congtru_menubaothuc == 4) cong_ngay_gio(&ngay4, &gio4);
            else if (congtru_menubaothuc == 5) cong_ngay_gio(&ngay5, &gio5);
          }
          else if (contro_bt == 9 || contro_bt == 10) /* Tháng or phút */
          {
            if (congtru_menubaothuc == 1) cong_thang_phut(&thang1, &phut1);
            else if (congtru_menubaothuc == 2) cong_thang_phut(&thang2, &phut2);
            else if (congtru_menubaothuc == 3) cong_thang_phut(&thang3, &phut3);
            else if (congtru_menubaothuc == 4) cong_thang_phut(&thang4, &phut4);
            else if (congtru_menubaothuc == 5) cong_thang_phut(&thang5, &phut5);
          }
        }
        HAL_Delay(150);
      }
      macdinh = gtxuong;
    }

    /* --- MENU button block --- */
    if (gtmenu != macdinh)    /* NÚT MENU */
    {
      if (gtmenu == 0) /* Khi nhấn nút */
      {
        demtong ++;

        if (demtong == 1) /* Ở menu tổng */
        {
          menu_tong();
        }
        else if (demtong == 2) /* Chọn các mục ở MENU TỔNG */
        {
          if(congtru_tong == 0){ /* Nhấn BACK từ Menu tổng về màn hình */
            demtong = 0;
            Get_Time();
            manhinh();
          }
          else if(congtru_tong == 1 || congtru_tong == 2) /* chọn menu tổng DATE or TIME */
            chonmenu_tong();
          else if(congtru_tong == 3) /* Menu BÁO THỨC */
            menu_baothuc();
        }

        else if (demtong == 4) /* Thoát CON TRỎ từ SET DATE or TIME ra */
        {
          if (congtru_tong == 1) /* Thoát CON TRỎ từ SET DATE ra */
          { demtong = 2; chonmenu_tong(); }
          else if (congtru_tong == 2) /* Thoát CON TRỎ từ SET TIME ra */
          { demtong = 2; chonmenu_tong(); }
          else if (congtru_tong == 3 && (contro_bt == 12 || contro_bt == 13 || contro_bt == 14 || contro_bt == 15))
          {
            menu_baothuc();
            demtong = 2;
            congtru_tong = 3;
            contro_bt = 6;
            hang = 0;
            lcd_cursor_off();
          }
        }

        else if (demtong == 3 && (congtru_tong == 2 || congtru_tong == 1) &&
                (contro == 12 || contro == 13 || contro == 14 || contro == 15))
        {
          demtong = 1;
          congtru_tong = 0;
          contro = 0;
          menu_tong();
          lcd_cursor_off();
        }

        else if (demtong == 3 && congtru_tong == 3) /* Chọn MENU Báo Thức  */
        {
          if(congtru_menubaothuc == 0){ /* từ Menu BÁO THỨC về Menu TỔNG */
            demtong = 1;
            congtru_menubaothuc = 0;
            menu_tong();
          }
          else if(congtru_menubaothuc == 1 || congtru_menubaothuc == 2 || congtru_menubaothuc == 3 ||
                  congtru_menubaothuc == 4 || congtru_menubaothuc == 5) /* chọn menu BT */
          {
            chonmenu_baothuc();
          }
        }

        else if (demtong == 5 && congtru_tong == 3 && (congtru_menubaothuc == 1 || congtru_menubaothuc == 2 ||
                congtru_menubaothuc == 3 || congtru_menubaothuc == 4 || congtru_menubaothuc == 5))  /* Từ chức năng SET ra */
        {
          chonmenu_baothuc();
          demtong = 3;
        }

        if(dembaothuc == 1){ /* Đang có BÁO THỨC */
          dembaothuc = 0;
          demtong = 0;
        }

        HAL_Delay(150);
      }
      macdinh = gtmenu;
    }

    /* Display logic */
    if (demtong == 0 && congtru_tong == 0) /* MÀN HÌNH */
    {
      Get_Time();
      manhinh();
      lcd_cursor_off();
    }
    else if ((demtong == 2 || demtong == 3) && congtru_tong != 3) /* Hiển thị SETUP DATE or TIME */
    {
      if(congtru_tong == 1) /* SET DATE */
      {
        if (ngay < 10){ lcd_put_cur(1,0); lcd_send_string("0"); snprintf(buffer,3,"%d",ngay); lcd_send_string(buffer); }
        else { lcd_put_cur(1,0); snprintf(buffer,3,"%d",ngay); lcd_send_string(buffer); }
        lcd_put_cur(1,2); lcd_send_string("/");
        if (thang < 10){ lcd_put_cur(1,3); lcd_send_string("0"); snprintf(buffer,3,"%d",thang); lcd_send_string(buffer); }
        else { lcd_put_cur(1,3); snprintf(buffer,3,"%d",thang); lcd_send_string(buffer); }
        lcd_put_cur(1,5); lcd_send_string("/");
        lcd_put_cur(1,6); snprintf(buffer,2,"%d",namng); lcd_send_string(buffer);
        lcd_put_cur(1,7); snprintf(buffer,2,"%d",namtr); lcd_send_string(buffer);
        lcd_put_cur(1,8); snprintf(buffer,2,"%d",namch); lcd_send_string(buffer);
        lcd_put_cur(1,9); snprintf(buffer,2,"%d",namdv); lcd_send_string(buffer);
        namtong = (namng * 1000) + (namtr * 100) + (namch * 10) + namdv;

        lcd_put_cur(1,contro);
        lcd_cursor_on();
        HAL_Delay(50);

        /* update RTC each cycle as in original (they called rtc.setDate in display) */
        Set_Time(time.seconds, time.minutes, time.hour, time.dayofweek, ngay, thang, (uint8_t)(namtong % 100));
      }
      else if (congtru_tong == 2) /* SET TIME */
      {
        if (gio < 10){ lcd_put_cur(1,0); lcd_send_string("0"); snprintf(buffer,3,"%d",gio); lcd_send_string(buffer); }
        else { lcd_put_cur(1,0); snprintf(buffer,3,"%d",gio); lcd_send_string(buffer); }
        lcd_put_cur(1,2); lcd_send_string(":");
        if (phut < 10){ lcd_put_cur(1,3); lcd_send_string("0"); snprintf(buffer,3,"%d",phut); lcd_send_string(buffer); }
        else { lcd_put_cur(1,3); snprintf(buffer,3,"%d",phut); lcd_send_string(buffer); }
        lcd_put_cur(1,5); lcd_send_string(":");
        if (giay < 10){ lcd_put_cur(1,6); lcd_send_string("0"); snprintf(buffer,3,"%d",giay); lcd_send_string(buffer); }
        else { lcd_put_cur(1,6); snprintf(buffer,3,"%d",giay); lcd_send_string(buffer); }

        lcd_put_cur(1,contro);
        lcd_cursor_on();
        HAL_Delay(50);

        /* FIXED: DS3231 expects seconds, minutes, hours. Previously parameters were passed as (hours,minutes,seconds),
           causing incorrect writes (e.g. month/day/year appearing wrong after save). Send in correct order here. */
        Set_Time((uint8_t)giay, (uint8_t)phut, (uint8_t)gio, time.dayofweek, time.dayofmonth, time.month, time.year);
      }
    }
    else if ((demtong == 3 || demtong == 4) && congtru_tong == 3) /* Hiển thị SETUP BÁO THỨC */
    {
      if(congtru_menubaothuc == 1) hien_thi_bt(&ngay1, &thang1, &gio1, &phut1);
      else if(congtru_menubaothuc == 2) hien_thi_bt(&ngay2, &thang2, &gio2, &phut2);
      else if(congtru_menubaothuc == 3) hien_thi_bt(&ngay3, &thang3, &gio3, &phut3);
      else if(congtru_menubaothuc == 4) hien_thi_bt(&ngay4, &thang4, &gio4, &phut4);
      else if(congtru_menubaothuc == 5) hien_thi_bt(&ngay5, &thang5, &gio5, &phut5);
    }

    /* Alarm check only when on main screen */
    if (demtong == 0) {
      if ((time.dayofmonth == ngay1 || time.dayofmonth == ngay2 || time.dayofmonth == ngay3 || time.dayofmonth == ngay4 || time.dayofmonth == ngay5) &&
          (time.month == thang1 || time.month == thang2 || time.month == thang3 || time.month == thang4 || time.month == thang5) &&
          (time.hour == gio1 || time.hour == gio2 || time.hour == gio3 || time.hour == gio4 || time.hour == gio5) &&
          (time.minutes == phut1 || time.minutes == phut2 || time.minutes == phut3 || time.minutes == phut4 || time.minutes == phut5) && time.seconds == 1)
      { dembaothuc = 1; }
    } else dembaothuc = 0;

    /* NEW: handle alarm with user-stop and timeout support */
    if (dembaothuc == 1) {
      handle_alarm_event();
    }

    /* Optionally print debug on UART if enabled */
    HAL_Delay(2); /* small idle */
  } /* end main loop */
}

/* MX_GPIO_Init: configure PA4/PA5/PA6 = INPUT_PULLUP and PA8 = output */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Buzzer PA8 */
  GPIO_InitStruct.Pin = BUZZER_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);

  /* Buttons PA4, PA5, PA6 as input pull-up */
  GPIO_InitStruct.Pin = BTN_MENU_PIN | BTN_UP_PIN | BTN_DOWN_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* MX_I2C1_Init kept simple; if project already has one, remove to avoid duplicate symbol */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) { /* Use existing Error_Handler in your project */ }
}

/* End of file */
