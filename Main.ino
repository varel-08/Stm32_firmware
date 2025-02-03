#include "stm32f1xx_hal.h"
#include "ssd1306.h"
#include "ff.h"
#include "gps.h"

// Pin konfigurasi
#define BUTTON_1_PIN GPIO_PIN_0 // PA0
#define BUTTON_2_PIN GPIO_PIN_1 // PA1
#define BUTTON_3_PIN GPIO_PIN_2 // PA2

// Variabel global
UART_HandleTypeDef huart1;
SPI_HandleTypeDef hspi1;
FATFS fs;							// File system untuk microSD
FIL file;							// File handler untuk microSD
char gps_data[128];					// Data GPS untuk display dan log
char user_name[] = "Ubur Ubur Project"; // Nama yang ingin ditampilkan

uint8_t logging = 0;	  // Status logging (0 = off, 1 = on)
uint8_t display_mode = 0; // Mode tampilan: 0 = koordinat, 1 = kecepatan, 2 = waktu

// Fungsi untuk inisialisasi komponen (GPS, OLED, SD)
void System_Init()
{
	HAL_Init();
	SystemClock_Config();

	// Inisialisasi GPIO untuk tombol
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitStruct.Pin = BUTTON_1_PIN | BUTTON_2_PIN | BUTTON_3_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// Inisialisasi UART untuk GPS
	__HAL_RCC_USART1_CLK_ENABLE();
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart1);

	// Inisialisasi OLED
	ssd1306_Init();
	ssd1306_Clear();
	ssd1306_UpdateScreen();

	// Inisialisasi SD card
	if (f_mount(&fs, "", 1) != FR_OK)
	{
		// Error mounting SD card
		while (1)
			;
	}
}

// Fungsi untuk membaca data GPS
void GPS_Read()
{
	// Simulasi pembacaan data GPS dan menyiapkan string GPS
	snprintf(gps_data, sizeof(gps_data), "Lat: 12.3456, Long: 65.4321, Speed: 100 km/h");
}

// Fungsi untuk menampilkan data pada OLED
void Display_Data()
{
	ssd1306_Clear();

	// Menampilkan nama pengguna di bagian atas layar
	ssd1306_SetCursor(0, 0);
	ssd1306_WriteString(user_name, Font_7x10, White);

	// Tampilan data GPS atau informasi lainnya berdasarkan mode
	if (display_mode == 0)
	{
		ssd1306_SetCursor(0, 12);
		ssd1306_WriteString("GPS: " gps_data, Font_7x10, White);
	}
	else if (display_mode == 1)
	{
		ssd1306_SetCursor(0, 12);
		ssd1306_WriteString("Speed: 100 km/h", Font_7x10, White);
	}
	else
	{
		ssd1306_SetCursor(0, 12);
		ssd1306_WriteString("Time: 12:34:56", Font_7x10, White);
	}

	ssd1306_UpdateScreen();
}

// Fungsi untuk menyimpan data ke microSD
void Save_Data()
{
	if (logging)
	{
		// Buka file log di microSD
		f_open(&file, "log.txt", FA_OPEN_APPEND | FA_WRITE);
		f_write(&file, gps_data, strlen(gps_data), NULL);
		f_write(&file, "\n", 1, NULL);
		f_close(&file);
	}
}

// Fungsi untuk memeriksa tombol
void Check_Buttons()
{
	if (HAL_GPIO_ReadPin(GPIOA, BUTTON_1_PIN) == GPIO_PIN_SET)
	{
		logging = !logging;
		HAL_Delay(200); // Debounce
	}

	if (HAL_GPIO_ReadPin(GPIOA, BUTTON_2_PIN) == GPIO_PIN_SET)
	{
		display_mode = (display_mode + 1) % 3;
		HAL_Delay(200); // Debounce
	}

	if (HAL_GPIO_ReadPin(GPIOA, BUTTON_3_PIN) == GPIO_PIN_SET)
	{
		// Tombol 3 bisa digunakan untuk mode lain atau reset tampilan
		HAL_Delay(200); // Debounce
	}
}

int main(void)
{
	System_Init();

	while (1)
	{
		GPS_Read();
		Display_Data();
		Save_Data();
		Check_Buttons();
		HAL_Delay(1000); // Update setiap detik
}
