#include <stdio.h>
#include <string.h>

#include "py/mpstate.h"
#include "usb.h"
#include "uart.h"
#include "Arduino.h"
#include MICROPY_HAL_H

uint32_t HAL_GetTick(void) {
  return millis();
}

void HAL_Delay(uint32_t Delay) {
  delay(Delay);
}

void mp_hal_set_interrupt_char(int c) {
  // The teensy 3.1 usb stack doesn't currently have the notion of generating
  // an exception when a certain character is received. That just means that
  // you can't press Control-C and get your python script to stop.
}

int mp_hal_stdin_rx_chr(void) {
    for (;;) {
        byte c;
        if (usb_vcp_recv_byte(&c) != 0) {
            return c;
        } else if (MP_STATE_PORT(pyb_stdio_uart) != NULL && uart_rx_any(MP_STATE_PORT(pyb_stdio_uart))) {
            return uart_rx_char(MP_STATE_PORT(pyb_stdio_uart));
        }
        __WFI();
    }
}

void mp_hal_stdout_tx_str(const char *str) {
    mp_hal_stdout_tx_strn(str, strlen(str));
}

void mp_hal_stdout_tx_strn(const char *str, uint32_t len) {
    if (MP_STATE_PORT(pyb_stdio_uart) != NULL) {
        uart_tx_strn(MP_STATE_PORT(pyb_stdio_uart), str, len);
    }
    if (usb_vcp_is_enabled()) {
        usb_vcp_send_strn(str, len);
    }
}

void mp_hal_stdout_tx_strn_cooked(const char *str, uint32_t len) {
    // send stdout to UART and USB CDC VCP
    if (MP_STATE_PORT(pyb_stdio_uart) != NULL) {
        uart_tx_strn_cooked(MP_STATE_PORT(pyb_stdio_uart), str, len);
    }
    if (usb_vcp_is_enabled()) {
        usb_vcp_send_strn_cooked(str, len);
    }
}
