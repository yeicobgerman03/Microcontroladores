#include <stdio.h>
#include <stdlib.h>

#define ESTADO_INIT      0
#define ESTADO_CERRADO   1
#define ESTADO_ABIERTO   2
#define ESTADO_CERRANDO  3
#define ESTADO_ABRIENDO  4
#define ESTADO_ERROR     5
#define TRUE  1
#define FALSE 0
#define RT_MAX 180
#define ERROR_LS 1
#define ERROR_OK 0
#define ERROR_RT 2

int ESTADO_SIGUIENTE = ESTADO_INIT;
int ESTADO_ACTUAL    = ESTADO_INIT;
int ESTADO_ANTERIOR  = ESTADO_INIT;

struct DATA_IO {
    unsigned int LSC:1;
    unsigned int LSA:1;
    unsigned int SPP:1;
    unsigned int MA:1;
    unsigned int MC:1;
    unsigned int Cont_RT;
    unsigned int led_A:1;
    unsigned int led_C:1;
    unsigned int led_ER:1;
    unsigned int COD_ERR;
    unsigned int DATOS_READY:1;
} data_io;

void delay() {
    // Simula un retardo; implementa la función según tus necesidades.
}

int Func_INIT(void) {
    ESTADO_ANTERIOR = ESTADO_INIT;
    ESTADO_ACTUAL = ESTADO_INIT;

    data_io.MC = FALSE;
    data_io.MA = FALSE;
    data_io.SPP = FALSE;
    data_io.led_C = TRUE;
    data_io.led_A = TRUE;
    data_io.led_ER = TRUE;
    delay();
    data_io.led_C = FALSE;
    data_io.led_A = FALSE;
    data_io.led_ER = FALSE;
    data_io.COD_ERR = FALSE;
    data_io.Cont_RT = 0;
    data_io.DATOS_READY = FALSE;

    while (!data_io.DATOS_READY); // Espera hasta que los datos estén listos.

    for (;;) {
        if (data_io.LSC == TRUE && data_io.LSA == FALSE) {
            return ESTADO_CERRADO;
        }
        if (data_io.LSC == TRUE && data_io.LSA == TRUE) {
            return ESTADO_ERROR;
        }
        if ((data_io.LSC == FALSE && data_io.LSA == FALSE) || (data_io.LSC == FALSE && data_io.LSA == TRUE)) {
            return ESTADO_CERRANDO;
        }
    }
}

int Func_CERRADO(void) {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRADO;
    data_io.MC = FALSE;
    data_io.SPP = FALSE;
    data_io.led_C = FALSE;
    data_io.led_A = FALSE;
    data_io.led_ER = FALSE;

    for (;;) {
        if (data_io.SPP == TRUE) {
            data_io.SPP = FALSE;
            return ESTADO_ABRIENDO;
        }
    }
}

int Func_CERRANDO(void) {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRANDO;
    data_io.MC = TRUE;
    data_io.Cont_RT = 0;
    data_io.led_C = TRUE;
    data_io.led_A = FALSE;
    data_io.led_ER = FALSE;

    for (;;) {
        if (data_io.LSC == TRUE) {
            return ESTADO_CERRADO;
        }
        if (data_io.Cont_RT > RT_MAX) {
            return ESTADO_ERROR;
        }
    }
}

int Func_ABRIENDO(void) {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABRIENDO;
    data_io.MA = TRUE;  // Activa el motor de apertura.
    data_io.Cont_RT = 0;
    data_io.led_A = TRUE;
    data_io.led_C = FALSE;
    data_io.led_ER = FALSE;

    for (;;) {
        if (data_io.LSA == TRUE) {
            data_io.MA = FALSE;
            return ESTADO_ABIERTO;
        }
        if (data_io.Cont_RT > RT_MAX) {
            data_io.MA = FALSE;
            return ESTADO_ERROR;
        }
    }
}

int Func_ABIERTO(void) {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABIERTO;
    data_io.MA = FALSE;
    data_io.SPP = FALSE;
    data_io.led_A = TRUE;
    data_io.led_C = FALSE;
    data_io.led_ER = FALSE;

    for (;;) {
        if (data_io.SPP == TRUE) {
            data_io.SPP = FALSE;
            return ESTADO_CERRANDO;
        }
    }
}

int Func_ERROR(void) {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ERROR;
    data_io.MA = FALSE;
    data_io.MC = FALSE;
    data_io.led_ER = TRUE;  // Indica error con el LED de error.
    data_io.COD_ERR = ERROR_RT;

    for (;;) {
        if (data_io.LSC == TRUE && data_io.LSA == FALSE) {
            data_io.led_ER = FALSE;
            data_io.COD_ERR = ERROR_OK;
            return ESTADO_CERRADO;
        }
    }
}

int main() {
    for (;;) {
        switch (ESTADO_SIGUIENTE) {
            case ESTADO_INIT:
                ESTADO_SIGUIENTE = Func_INIT();
                break;
            case ESTADO_CERRADO:
                ESTADO_SIGUIENTE = Func_CERRADO();
                break;
            case ESTADO_CERRANDO:
                ESTADO_SIGUIENTE = Func_CERRANDO();
                break;
            case ESTADO_ABRIENDO:
                ESTADO_SIGUIENTE = Func_ABRIENDO();
                break;
            case ESTADO_ABIERTO:
                ESTADO_SIGUIENTE = Func_ABIERTO();
                break;
            case ESTADO_ERROR:
                ESTADO_SIGUIENTE = Func_ERROR();
                break;
            default:
                ESTADO_SIGUIENTE = ESTADO_ERROR;
                break;
        }
    }
    return 0;
}
