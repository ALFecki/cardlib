#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Группа данных для чтения из КПСИС
 */
typedef enum DataGroup {
  Dg1 = (1 << 0),
  Dg2 = (1 << 1),
  Dg3 = (1 << 2),
  Dg4 = (1 << 3),
  Dg5 = (1 << 4),
} DataGroup;

/**
 * Права доступа к прикладной программе eID
 */
typedef enum EidAccess {
  /**
   * Право проверки возраста
   */
  AgeVerification = (1 << 0),
  /**
   * Возможность управления PIN
   */
  PinControl = (1 << 5),
  /**
   * Право чтения DG1
   */
  DG1 = (1 << 8),
  /**
   * Право чтения DG2
   */
  DG2 = (1 << 9),
  /**
   * Право чтения DG3
   */
  DG3 = (1 << 10),
  /**
   * Право чтения DG4
   */
  DG4 = (1 << 11),
  /**
   * Право чтения DG5
   */
  DG5 = (1 << 12),
} EidAccess;

/**
 * Права доступа к прикладной программе eSign
 */
typedef enum EsignAccess {
  /**
   * Право изменять пароль PIN
   */
  ChangePin = (1 << 7),
  /**
   * Право разблокировать пароль PIN
   */
  UnlockPin = (1 << 8),
  /**
   * Право вырабатывать подпись, разбирать токен ключа, читать сертификаты в терминальном режиме
   */
  TerminalMode = (1 << 14),
  /**
   * Право вырабатывать подпись, разбирать токен ключа, читать сертификаты в базовом режиме
   */
  BasicMode = (1 << 15),
} EsignAccess;

/**
 * Уровень логирования
 */
typedef enum LogLevel {
  Trace,
  Debug,
  Info,
  Warn,
  Error,
} LogLevel;

/**
 * Контекст защищенного соединения, КП-КТА или КТА-Терминал
 */
typedef enum SecureContext {
  BAUTH = 0,
  BPACE = 1,
} SecureContext;

/**
 * Структура для хранения сертификатов ГОССУОК, валидации маршрутов сертификации.
 *
 * При ее заполнении, представленные сертификаты должны предоставлять полный маршрут сертификации.
 */
typedef struct CertificateCtx CertificateCtx;

/**
 * Структура, реализующая весь необходимый функционал для взаимодействия с КТА в формате ID-Карты
 *
 * # Поля:
 * * `rw_ctx`: Интерфейс для получения/отправки информации с/на ID-Карту
 * * `cert_ctx`: [`CertificateCtx`]
 * * `sec_object`: [`SecurityObject`]
 * * `sec_ctx`: Реализация защищенного соединения для взаимодействия с КТА
 * * `auth_ctx`: Авторизационный провайдер
 */
typedef struct IdCard IdCard;

typedef struct IdCard IdCard;

/**
 * Указатель на участок памяти + его длина, аналог [`Vec<u8>`] для ffi
 */
typedef struct Data {
  /**
   * Длина
   */
  uint32_t len;
  /**
   * Указатель на первый элемент буфера
   */
  uint8_t *data;
} Data;

/**
 * Callback, используемый для I/O операций
 * * `ctx_data`: Контекст, используемый для общения с устройством
 * * `data`: Данные, которые требуется передать
 * * `response`: Указатель на участок памяти, в который будет записан ответ
 * * i32 - код ошибки
 */
typedef int32_t (*TransmitFunctionRaw)(const void *ctx_data, const struct Data *data, struct Data *response);

/**
 * Создание контекста для взаимодействия с ID-Картой \
 * При создании контекста выполняется выбор апплета КТА + обязательное самотестирование
 *
 * # Условия использования
 * * Использование возможно из любого состояния
 *
 * # Arguments
 *
 * * `ctx_data`: Контекст для I/O операций
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `cert_ctx`: Структура сертификатов ГОССУОК
 * * `tf`: Callback для I/O операций
 * * `log_level`: Уровень логирования
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * int32_t transmit(const void *ctx_data, const Data *data, Data *response) {
 *     SCARDHANDLE hCard = *(SCARDHANDLE *) ctx_data;
 *     SCARD_IO_REQUEST pioSendPci = *SCARD_PCI_T1;
 *
 *     uint32_t len = 264;
 *     unsigned char *buf = (unsigned char *) malloc(len);
 *
 *     int32_t rv = SCardTransmit(hCard, &pioSendPci,
 *                                data->data,
 *                                data->len,
 *                                NULL,
 *                                (unsigned char *) buf,
 *                                &len);
 *
 *     response->len = len;
 *     memcpy(response->data, buf, len);
 *     CHECK("SCardTransmit", rv);
 *     free(buf);
 *     return rv;
 * }
 * //...
 * //Здесь hCard - SCARDHANDLE PCSC-Lite
 * auto certs = get_certs();
 * void *ctx = nullptr;
 * int32_t err = id_kta_init_raw(&hCard, &ctx, &certs, transmit, Trace);
 * if(err != 0) {
 *     printf("Error: %d", err);
 * }
 * ```
 */
int32_t id_kta_init_raw(const void *ctx_data,
                        const IdCard **ctx_card,
                        struct CertificateCtx *cert_ctx,
                        TransmitFunctionRaw tf,
                        enum LogLevel log_level);

/**
 * Создание контекста для хранения сертификатов ГОССУОК
 *
 * # Arguments
 *
 * * `ctx`: Контекст для хранения и проверки сертификатов
 *
 * returns: ()
 *
 * # Examples
 *
 * ```C
 * CertificateCtx *pCertificateCtx;
 * id_kta_create_cert_ctx((const struct CertificateCtx **) &pCertificateCtx);
 * ```
 */
void id_kta_create_cert_ctx(const struct CertificateCtx **ctx);

/**
 * Добавление в контекст сертификата открытого ключа ГОССУОК
 *
 * # Arguments
 *
 * * `ctx`: Контекст для хранения и проверки сертификатов
 * * `ca`: Сертификат открытого ключа ГОССУОК
 *
 * returns: [`i32`]
 *
 * # Examples
 * ```C
 * void read_file(const char *filename, Data *data) {
 *     FILE *f = fopen(filename, "rb");
 *     if (!f) {
 *         printf("Could not open file %s\n", filename);
 *         exit(1);
 *     }
 *     fseek(f, 0, SEEK_END);
 *     data->len = ftell(f);
 *     fseek(f, 0, SEEK_SET);
 *     data->data = (unsigned char *) malloc(data->len);
 *     fread(data->data, 1, data->len, f);
 *     fclose(f);
 * }
 *
 * Data cert;
 * read_file("ruc.cer", &cert);
 *
 * CertificateCtx *pCertificateCtx;
 * id_kta_create_cert_ctx((const struct CertificateCtx **) &pCertificateCtx);
 *
 * id_kta_cert_insert_ca(pCertificateCtx, &cert);
 * ```
 */
int32_t id_kta_cert_insert_ca(const struct CertificateCtx *ctx,
                              const struct Data *ca);

/**
 * Добавление в контекст списка отозванных сертификатов ГОССУОК
 *
 * # Arguments
 *
 * * `ctx`: Контекст для хранения и проверки сертификатов
 * * `crl`: Список отозванных сертификатов ГОССУОК
 *
 * returns: [`i32`]
 *
 * # Examples
 * ```C
 * void read_file(const char *filename, Data *data) {
 *     FILE *f = fopen(filename, "rb");
 *     if (!f) {
 *         printf("Could not open file %s\n", filename);
 *         exit(1);
 *     }
 *     fseek(f, 0, SEEK_END);
 *     data->len = ftell(f);
 *     fseek(f, 0, SEEK_SET);
 *     data->data = (unsigned char *) malloc(data->len);
 *     fread(data->data, 1, data->len, f);
 *     fclose(f);
 * }
 *
 * Data crl;
 * read_file("ruc.crl", &crl);
 *
 * CertificateCtx *pCertificateCtx;
 * id_kta_create_cert_ctx((const struct CertificateCtx **) &pCertificateCtx);
 *
 * id_kta_cert_insert_crl(pCertificateCtx, &crl);
 * ```
 */
int32_t id_kta_cert_insert_crl(const struct CertificateCtx *ctx,
                               const struct Data *crl);

/**
 * Выполнение самотестирования КТА \
 * При самотестировании проверяются симметричные и ассиметричные криптоалгоритмы на КТА
 *
 * # Условия использования
 * * Использование возможно из любого состояния
 * * Выбран мастер-файл
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * int32_t err_code = id_kta_selftest(ctx_id_card);
 * if(err_code != 0) {
 *     printf("Error: %d", err_code);
 * }
 * ```
 */
int32_t id_kta_selftest(const IdCard *ctx_card);

/**
 * Выполнение самотестирования криптографических алгоритмов \
 * При самотестировании проверяются симметричные и ассиметричные криптоалгоритмы
 *
 * # Условия использования
 * * Использование возможно из любого состояния
 *
 * # Arguments
 *
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * int32_t err_code = id_kta_crypto_selftest();
 * if(err_code != 0) {
 *     printf("Error: %d", err_code);
 * }
 * ```
 */
int32_t id_kta_crypto_selftest(void);

/**
 * Выполнение авторизации по CAN коду и создание защищенного соединения КТА - КП
 *
 * # Условия использования
 * * Использование возможно из любого состояния
 * * При повторной авторизации, открытое защищенное соединение будет закрыто
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `can`: CAN-код
 * * `eid_access`: Права доступа к прикладной программе eID
 *
 * returns: [`i32`]
 *
 * # Examples
 * ```C
 * char can[] = "609015";
 * int16_t eid_rules = EidAccess::DG1 | EidAccess::DG2;
 * int32_t res = id_kta_can_auth(ctx_id_card, can, eid_access);
 * if(err_code != 0) {
 *     printf("Error: %d", err_code);
 * }
 * ```
 */
int32_t id_kta_can_auth(IdCard *ctx_card,
                        const char *can,
                        int16_t eid_access);

/**
 * Выполнение авторизации по PIN1 коду и создание защищенного соединения КТА - КП
 * # Arguments
 *
 * # Условия использования
 * * Использование возможно из любого состояния
 * * При повторной авторизации, открытое защищенное соединение будет закрыто
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `pin`: PIN1-код
 * * `eid_access`: Права доступа к прикладной программе eID
 * * `esign_access`: Права доступа к прикладной программе eSign
 * * `sign_count`: Количество подписей, которые можно выработать без повторного подтверждения флага PIN2
 *
 * returns: [`i32`]
 *
 * # Examples
 * ```C
 * char pin[] = "4270018";
 * int16_t eid_rules = EidAccess::DG1 | EidAccess::DG2;
 * // Если значение отрицательное - права доступа к ПП отсутствуют.
 * int16_t esign_rules = -1;
 * int32_t err_code = id_kta_pin_auth(ctx_id_card, pin, eid_access, esign_access, 0);
 * if(err_code != 0) {
 *     printf("Error: %d", err_code);
 * }
 * ```
 */
int32_t id_kta_pin_auth(IdCard *ctx_card,
                        const char *pin,
                        int16_t eid_access,
                        int32_t esign_access,
                        int8_t sign_count);

/**
 * Выполнение авторизации по PUK коду и создание защищенного соединения КТА - КП
 *
 * # Условия использования
 * * Использование возможно из любого состояния
 * * При повторной авторизации, открытое защищенное соединение будет закрыто
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `pin`: PIN1-код
 * * `eid_access`: Права доступа к прикладной программе eID
 * * `esign_access`: Права доступа к прикладной программе eSign
 * * `sign_count`: Количество подписей, которые можно выработать без повторного подтверждения флага PIN2
 *
 * returns: [`i32`]
 *
 * # Examples
 * ```C
 * char pin[] = "4270018";
 * int16_t eid_rules = EidAccess::DG1 | EidAccess::DG2;
 * // Если значение отрицательное - права доступа к ПП отсутствуют.
 * int16_t esign_rules = -1;
 * int32_t err_code = id_kta_puk_auth(ctx_id_card, pin, eid_access, esign_access, 0);
 * if(err_code != 0) {
 *     printf("Error: %d", err_code);
 * }
 * ```
 */
int32_t id_kta_puk_auth(IdCard *ctx_card,
                        const char *puk,
                        int16_t eid_access,
                        int32_t esign_access,
                        int8_t sign_count);

/**
 * Взаимная аутентификация КТА и терминала по протоколу BAUTH
 *
 * # Условия использования
 * * Использование возможно после авторизации по PIN/PUK коду
 * * При повторной авторизации, открытое защищенное соединение будет закрыто
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `kpsis_url`: URL КПСИС в формате scheme://host:port
 *
 * returns: [`i32`]
 *
 * # Examples
 * ```C
 * IdCard *pIdCard;
 * int32_t err = id_kta_init_raw(&hCard, (const IdCard **) &pIdCard, pCertificateCtx, transmit, Trace);
 * int16_t eid_access = DG1 | DG2 | DG3 | DG4 | DG5 | PinControl | AgeVerification;
 * int32_t esign_access = ChangePin | UnlockPin | TerminalMode | BasicMode;
 * err |= id_kta_pin_auth(pIdCard, "981511", eid_access, esign_access, 0);
 * err |= id_kta_select_esign(pIdCard);
 * err |= id_kta_pin2_verify(pIdCard, "8416241");
 * err |= id_kta_select_mf(pIdCard);
 * err |= id_kta_terminal_auth(pIdCard, "http://localhost:48777");
 * if(err != 0) {
 *     printf("Error: %d", err_code);
 * }
 * ```
 */
int32_t id_kta_terminal_auth(IdCard *ctx_card,
                             const char *kpsis_url);

/**
 * Метод для реализации /api/v1/bauth для интеграции с ЕСИФЮЛ
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст данных карты
 * * `json`: JSON строка, содержащая поля `data` и `init` строкового типа
 * * `output`: Указатель на область памяти, куда будет записан ответ в JSON, содержащий поля `SO_CERT` и `cert_id` строкового типа
 *
 * returns: i32
 *
 * # Examples
 *
 * ```C
 * Data *output_callback;
 * char json[] = "{\"data\":\"AA==\",\"init\":\"true\"}";
 * int32_t err = id_kta_api_bauth_callback(ctx_card, &json, &output_callback);
 * ```
 */
int32_t id_kta_api_bauth_callback(const IdCard *ctx_card,
                                  const char *json,
                                  const struct Data **output);

/**
 * Метод для реализации /api/v1/terminal_proxy_bauth_init для интеграции с ЕСИФЮЛ
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст данных карты
 * * `json`: JSON строка, содержащая поля `cmd_to_card`, и `terminal_certificate` строкового типа, а также `is_bilateral` булевого типа
 * * `output`: Указатель на область памяти, куда будет записан ответ в JSON, содержащий поля `card_response` и `err` строкового типа
 *
 * returns: i32
 *
 * # Examples
 *
 * ```C
 * Data *output_callback;
 * int32_t err = id_kta_api_terminal_proxy_bauth_init_callback(ctx_card, &json, &output_callback);
 * ```
 */
int32_t id_kta_api_terminal_proxy_bauth_init_callback(const IdCard *ctx_card,
                                                      const char *json,
                                                      const struct Data **output);

/**
 * Метод для реализации /api/v1/terminal_proxy_command и /api/v1/terminal_proxy_bauth для интеграции с ЕСИФЮЛ
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст данных карты
 * * `json`: JSON строка, содержащая поля `cmd_to_card`, и `header_cmd_to_card` строкового типа
 * * `output`: Указатель на область памяти, куда будет записан ответ в JSON, содержащий поля `card_response` и `err` строкового типа
 *
 * returns: i32
 *
 * # Examples
 *
 * ```C
 * Data *output_callback;
 * int32_t err = id_kta_api_terminal_proxy_command_callback(ctx_card, &json, &output_callback);
 * ```
 */
int32_t id_kta_api_terminal_proxy_command_callback(const IdCard *ctx_card,
                                                   const char *json,
                                                   const struct Data **output);

/**
 * Переключение между соединениями
 *
 * # Условия использования
 * * Использование возможно после авторизации по PIN/PUK коду
 * * Выполнена взаимная аутентификация КТА - Терминал
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `ctx`: Тип защищенного соединения
 *
 * returns: [`i32`]
 *
 * # Examples
 * ```C
 * id_kta_switch_secure_ctx(pIdCard, Bauth);
 * ```
 */
int32_t id_kta_switch_secure_ctx(const IdCard *ctx_card,
                                 enum SecureContext ctx);

/**
 * Получение групп данных в терминальном режиме в формате JSON
 *
 * # Условия использования
 * * Использование возможно после авторизации по PIN/PUK коду
 * * Использование возможно только после установления защищенного соединение КТА - Терминал
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `dg_to_read`: Битовая маска групп данных для чтения
 * * `data_group`: Полученные группы данных в формате JSON
 *
 * returns: [`i32`]
 *
 * # Examples
 * ```C
 * uint8_t dg_to_read = Dg1 | Dg2 | Dg3 | Dg4 | Dg5;
 * Data *dg;
 * id_kta_terminal_get_dg(pIdCard, dg_to_read, (const struct Data **) &dg);
 * for(int i = 0; i < dg->len; i++) {
 *     printf("%c", dg->data[i]);
 * }
 * ```
 */
int32_t id_kta_terminal_get_dg(const IdCard *ctx_card,
                               uint8_t dg_to_read,
                               const struct Data **data_group);

/**
 * Получение подписи в формате CMS в терминальном режиме (base64)
 *
 * # Условия использования
 * * Использование возможно после авторизации по PIN/PUK коду
 * * Использование возможно только после установления защищенного соединение КТА - Терминал
 * * Флаг подтверждения PIN2 установлен
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `data`: Данные для подписания
 * * `signature`: Выработанная CMS в формате base64
 *
 * returns: [`i32`]
 *
 * # Examples
 * ```C
 * Data to_sign = {
 *     .data = (unsigned char *) "Hello World",
 *     .len = strlen("Hello World")
 * };
 * Data *signature;
 * id_kta_terminal_sign(pIdCard, &to_sign, (const struct Data **) &signature);
 * ```
 */
int32_t id_kta_terminal_sign(const IdCard *ctx_card,
                             const struct Data *data,
                             const struct Data **signature);

/**
 * Установка подтверждения флага PIN2 \
 * Подтверждение флага PIN2 требуется для выработки ЭЦП
 *
 * # Условия использования
 * * Использование возможно после аутентификации по PIN/PUK
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `pin2`: PIN2
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * char pin2[] = "4270018";
 * int32_t res  = id_kta_pin2_verify(ctx_id_card, pin2);
 * if(res != 0) {
 *     printf("Error: %d", err_code);
 * }
 * ```
 */
int32_t id_kta_pin2_verify(const IdCard *ctx_card,
                           const char *pin2);

/**
 * Проверка флага подтверждения PIN2
 *
 * # Условия использования
 * * Использование возможно после аутентификации по PIN/PUK
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * int32_t res = id_kta_pin2_check(ctx_id_card);
 * //6A82 - не установлен
 * //9000 - установлен
 * if(res != 0) {
 *     printf("result: %d", res);
 * }
 * ```
 */
int32_t id_kta_pin2_check(const IdCard *ctx_card);

/**
 * Разблокировка PIN1
 *
 * # Условия использования
 * * Использование возможно после аутентификации по PIN/PUK
 * * Заданы права доступа для разблокировки PIN
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `pin`: Новый PIN1, для смены. В случае передачи пустой строки "" пароль будет разблокирован без смены.
 *
 * returns: [`i32`]
 *
 * # Examples
 * ## Со сменой PIN
 * ```C
 * char new_pin[] = "123123";
 * int32_t res = id_kta_unblock_pin1(ctx_id_card, new_pin);
 * if res != 0x9000 {
 *     printf("Error: %d", res);
 * }
 * ```
 * ## Без смены PIN
 * ```C
 * int32_t res = id_kta_unblock_pin1(ctx_id_card, "");
 * if(res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_unblock_pin1(const IdCard *ctx_card,
                            const char *pin);

/**
 * Разблокировка PIN2
 *
 * # Условия использования
 * * Использование возможно после аутентификации по PIN/PUK
 * * Заданы права доступа для разблокировки PIN
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `pin`: Новый PIN2, для смены. В случае передачи пустой строки "" пароль будет разблокирован без смены.
 *
 * returns: [`i32`]
 *
 * # Examples
 * ## Со сменой PIN
 * ```C
 * char new_pin[] = "12312123";
 * int32_t res = id_kta_unblock_pin2(ctx_id_card, new_pin);
 * if res != 0x9000 {
 *     printf("Error: %d", res);
 * }
 * ```
 * ## Без смены PIN
 * ```C
 * int32_t res = id_kta_unblock_pin2(ctx_id_card, "");
 * if(res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_unblock_pin2(const IdCard *ctx_card,
                            const char *pin2);

/**
 * Смена PIN1
 *
 * # Условия использования
 * * Использование возможно после аутентификации по PIN/PUK
 * * Заданы права доступа для смены PIN
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `old_pin`: Старый PIN1
 * * `new_pin`: Новый PIN1
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * char old_pin[] = "4270018";
 * char new_pin[] = "4270019";
 * int32_t res = id_kta_change_pin1(ctx_id_card, old_pin, new_pin);
 * if(res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_change_pin1(IdCard *ctx_card,
                           const char *old_pin,
                           const char *new_pin);

/**
 * Смена PIN2
 *
 * # Условия использования
 * * Использование возможно после аутентификации по PIN/PUK
 * * Заданы права доступа для смены PIN
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `old_pin`: Старый PIN1
 * * `new_pin`: Новый PIN1
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * char old_pin[] = "930025";
 * char new_pin[] = "930026";
 * int32_t res = id_kta_change_pin1(ctx_id_card, old_pin, new_pin);
 * if(res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_change_pin2(IdCard *ctx_card,
                           const char *old_pin,
                           const char *new_pin);

/**
 * Сброс флага подтверждения PIN2
 *
 * # Условия использования
 * * Использование возможно после аутентификации по PIN1/PUK
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * int32_t res = id_kta_clear_flag_pin2(ctx_id_card);
 * if(res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_clear_flag_pin2(const IdCard *ctx_card);

/**
 * Выбор прикладной программы eID
 *
 * # Условия использования
 * * Использование возможно после аутентификации по CAN/PIN1/PUK
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * int32_t res = id_kta_select_eid(ctx_id_card);
 * if(res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_select_eid(const IdCard *ctx_card);

/**
 * Получить данные DG1 в виде JSON
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по CAN/PIN1/PUK
 * * Выбрана прикладная программа eID
 * * Заданы права доступа к DG1
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `data_group`: Указатель на область памяти, в которую будет записана группа данных
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_dg1(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * for (int i = 0; i < buf->len; i++){
 *     printf("%c", buf->data[i]);
 * }
 * ```
 */
int32_t id_kta_get_dg1(const IdCard *ctx_card,
                       const struct Data **data_group);

/**
 * Получить данные DG2 в виде JSON
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по CAN/PIN1/PUK
 * * Выбрана прикладная программа eID
 * * Заданы права доступа к DG2
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `data_group`: Указатель на область памяти, в которую будет записана группа данных
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_dg2(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * for (int i = 0; i < buf->len; i++){
 *     printf("%c", buf->data[i]);
 * }
 * ```
 */
int32_t id_kta_get_dg2(const IdCard *ctx_card,
                       const struct Data **data_group);

/**
 * Получить данные DG3 в виде JSON
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по CAN/PIN1/PUK
 * * Выбрана прикладная программа eID
 * * Заданы права доступа к DG3
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `data_group`: Указатель на область памяти, в которую будет записана группа данных
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_dg3(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * for (int i = 0; i < buf->len; i++){
 *     printf("%c", buf->data[i]);
 * }
 * ```
 */
int32_t id_kta_get_dg3(const IdCard *ctx_card,
                       const struct Data **data_group);

/**
 * Получить данные DG4 в виде JSON
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по CAN/PIN1/PUK
 * * Выбрана прикладная программа eID
 * * Заданы права доступа к DG4
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `data_group`: Указатель на область памяти, в которую будет записана группа данных
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_dg4(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * for (int i = 0; i < buf->len; i++){
 *     printf("%c", buf->data[i]);
 * }
 * ```
 */
int32_t id_kta_get_dg4(const IdCard *ctx_card,
                       const struct Data **data_group);

/**
 * Получить данные DG5 в виде JSON
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по CAN/PIN1/PUK
 * * Выбрана прикладная программа eID
 * * Заданы права доступа к DG5
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `data_group`: Указатель на область памяти, в которую будет записана группа данных
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_dg5(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * for (int i = 0; i < buf->len; i++){
 *     printf("%c", buf->data[i]);
 * }
 * ```
 */
int32_t id_kta_get_dg5(const IdCard *ctx_card,
                       const struct Data **data_group);

/**
 * Выбор апплета КТА
 *
 * # Условия использования
 * * Использование возможно из любого состояния
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * int32_t res = id_kta_select_kta(ctx_id_card);
 * if(res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_select_kta(const IdCard *ctx_card);

/**
 * Выбор прикладной программы eSign
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * int32_t res = id_kta_select_esign(ctx_id_card);
 * if(res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_select_esign(const IdCard *ctx_card);

/**
 * Выбор мастер-файла
 *
 * # Условия использования
 * * Использование возможно из любого состояния
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * int32_t res = id_kta_select_mf(ctx_id_card);
 * if(res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_select_mf(const IdCard *ctx_card);

/**
 * Получить СОК базового режима хранящийся на КТА в формате ASN.1
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `cok`: Указатель на область памяти, в которую будет записан СОК
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```С
 * Data *buf;
 * int32_t res = id_kta_get_basic_cok(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_get_basic_cok(const IdCard *ctx_card,
                             const struct Data **cok);

/**
 * Получить публичный ключ из базового СОКа в формате ASN.1
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `key`: Указатель на область памяти, в которую будет записан открытый ключ
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```С
 * Data *buf;
 * int32_t res = id_kta_get_basic_cok_public_key(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_get_basic_cok_public_key(const IdCard *ctx_card,
                                        const struct Data **key);

/**
 * Получить серийный номер СОК базового режима хранящийся на КТА в формате ASN.1
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `cok`: Указатель на область памяти, в которую будет записан серийный номер сертификата
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_basic_cok_serial(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_get_basic_cok_serial(const IdCard *ctx_card,
                                    const struct Data **cok_serial);

/**
 * Получить хеш-значение открытого ключа СОК базового режима хранящийся на КТА
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `cok`: Указатель на область памяти, в которую будет записан хеш-идентификатор
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_basic_cok_hash(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_get_basic_cok_hash(const IdCard *ctx_card,
                                  const struct Data **key_hash);

/**
 * Получить СОК терминального режима хранящийся на КТА в формате ASN.1
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `cok`: Указатель на область памяти, в которую будет записан СОК
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_terminal_cok(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_get_terminal_cok(const IdCard *ctx_card,
                                const struct Data **cok);

/**
 * Получить серийный номер СОК терминального режима хранящийся на КТА в формате ASN.1
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `cok`: Указатель на область памяти, в которую будет записан серийный номер сертификата
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_terminal_cok_serial(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_get_terminal_cok_serial(const IdCard *ctx_card,
                                       const struct Data **cok_serial);

/**
 * Получить хеш-значение открытого ключа СОК терминального режима хранящийся на КТА
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `cok`: Указатель на область памяти, в которую будет записан хеш-идентификатор
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_terminal_cok_hash(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_get_terminal_cok_hash(const IdCard *ctx_card,
                                     const struct Data **key_hash);

/**
 * Получить СОК терминального режима хранящийся на КТА в формате ASN.1
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `cok`: Указатель на область памяти, в которую будет записан открытый ключ
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_terminal_public_key(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_get_terminal_public_key(const IdCard *ctx_card,
                                       const struct Data **key);

/**
 * Получить объект Name
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `name`: Указатель на область памяти, в которую будет записан объект Name
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_name(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_get_name(const IdCard *ctx_card,
                        const struct Data **name);

/**
 * Получить объект Extension
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `extension`: Указатель на область памяти, в которую будет записан объект Extension
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_get_extenson(ctx, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t id_kta_get_extension(const IdCard *ctx_card,
                             const struct Data **extension);

/**
 * Подписать данные с помощью КТА и вернуть подпись в формате 48 байт
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Установлены права доступа к базовому режиму eSign
 * * Установлен флаг подтверждения PIN2
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `data`: Данные, которые требуется подписать
 * * `signature`: Указатель на область памяти, в которую будет записана подпись
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 *
 * int32_t err_pin_auth = id_kta_pin_auth(ctx, "930025", -1, (int16_t)EsignAccess::BasicMode, 2);
 * if(err_pin_auth != 0) {
 *     printf("Error: %04X", err_pin_auth);
 * }
 * int32_t err_select_esign = id_kta_select_esign(ctx);
 * if(err_select_esign != 0) {
 *     printf("Error: %04X", err_pin_auth);
 * }
 * int32_t err_pin2_verify = id_kta_pin2_verify(ctx, "4270018");
 * if(err_pin2_verify != 0) {
 *     printf("Error: %04X", err_pin2_verify);
 * }
 * uint8_t data[] = {0x00, 0x00};
 * Data for_sign = Data {
 *     .len = 2,
 *     .data = data
 * };
 * Data *buf;
 * int32_t err_sign = id_kta_sign(ctx, &for_sign, &buf);
 * if (err_sign != 0) {
 *     printf("Error: %04X", err_sign);
 * }
 * for (int i = 0; i < buf->len; i++){
 *     printf("%02X", buf->data[i]);
 * }
 * ```
 */
int32_t id_kta_sign(const IdCard *ctx_card,
                    const struct Data *data,
                    const struct Data **signature);

/**
 * Подписать данные с помощью КТА и вернуть подпись в формате CMS
 *
 * # Условия использования
 * * Требуется предварительная аутентификация по PIN1/PUK
 * * Установлены права доступа к базовому режиму eSign
 * * Установлен флаг подтверждения PIN2
 * * Выбрана прикладная программа eSign
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `data`: Данные, которые требуется подписать
 * * `signature`: Указатель на область памяти, в которую будет записана подпись
 * * `acr_list`: *const *const u8,
 * * `acr_size`: Указатель на список размерностей переданных атрибутных сертификатов,
 * * `acr_count`: Количество переданных атрибутных сертификатов (0 если отсуствуют),
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 *
 * int32_t err_pin_auth = id_kta_pin_auth(ctx, "930025", -1, (int16_t)EsignAccess::BasicMode, 2);
 * if(err_pin_auth != 0) {
 *     printf("Error: %04X", err_pin_auth);
 * }
 * int32_t err_select_esign = id_kta_select_esign(ctx);
 * if(err_select_esign != 0) {
 *     printf("Error: %04X", err_pin_auth);
 * }
 * int32_t err_pin2_verify = id_kta_pin2_verify(ctx, "4270018");
 * if(err_pin2_verify != 0) {
 *     printf("Error: %04X", err_pin2_verify);
 * }
 * uint8_t data[] = {0x00, 0x00};
 * Data for_sign = Data {
 *     .len = 2,
 *     .data = data
 * };
 * Data *buf;
 * int32_t err_sign = id_kta_sign_cms(ctx, &for_sign, &buf, nullptr, nullptr, 0);
 * if (err_sign != 0) {
 *     printf("Error: %04X", err_sign);
 * }
 * for (int i = 0; i < buf->len; i++){
 *     printf("%02X", buf->data[i]);
 * }
 * ```
 */
int32_t id_kta_sign_cms(const IdCard *ctx_card,
                        const struct Data *data,
                        const struct Data **signature,
                        const uint8_t *const *acr_list,
                        const uintptr_t *acr_size,
                        uintptr_t acr_count);

/**
 * Проверить структуру CMS
 *
 * # Условия использования
 * * Возможно использование из любого состояния
 *
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `signature`: Указатель на область памяти, где лежит CMS
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * int32_t res = id_kta_verify_cms(id_card_ctx, signature_data);
 * if(res!=0) {
 *     printf("Error: %02X");
 * }
 * ```
 */
int32_t id_kta_verify_cms(const IdCard *card_ctx, const struct Data *signature);

/**
 * Проверка подписи в формате 48 байт
 *
 * # Arguments
 *
 * * `data`: Данные для проверки подписи
 * * `signature`: Подпись
 * * `key`: Ключ
 *
 * returns: [`i32`] \
 * 0 - корректная подпись \
 * 1 - подпись неверна \
 * Другое - ошибка
 *
 * # Examples
 *
 * ```C
 * id_kta_verify_signature(data, signature, key);
 * ```
 */
int32_t id_kta_verify_signature(const struct Data *data,
                                const struct Data *signature,
                                const struct Data *key);

/**
 * I/O для отправки команд на карту
 *
 * # Arguments
 *
 * * `card_ctx`: Контекст данных ID-карты
 * * `header`: CLA, INS, P1, P2 в формате BASE64
 * * `cmd`: CDF в формате BASE64
 *
 * returns: i32
 *
 * # Examples
 *
 * ```C
 * id_kta_execute_raw(pIdCard, &header, &cmd).
 * ```
 */
int32_t id_kta_execute_raw(const IdCard *card_ctx,
                           const struct Data *header,
                           const struct Data *cmd);

/**
 * Очистка памяти контекста библиотеки
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст библиотеки
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * id_kta_drop_ctx(ctx);
 * ```
 */
int32_t id_kta_drop_ctx(IdCard *ctx_card);

/**
 * Получить Belt-Hash от данных
 *
 * # Условия использования
 * * Использование доступно из любого состояния
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `data_group`: Указатель на область памяти, в которую будет записан хэш
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = belt_hash(data, &buf);
 * if (res != 0) {
 *     printf("Error: %d", res);
 * }
 * ```
 */
int32_t belt_hash(const struct Data *data,
                  const struct Data **output);

/**
 * Очистка памяти объектов типа [`Data`]
 *
 * # Arguments
 *
 * * `data`: Объект
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * id_kta_drop_data(data);
 * ```
 */
void id_kta_drop_data(struct Data *data);

/**
 * Получить информацию о разработчике и сборке
 *
 * # Условия использования
 * * Использование доступно из любого состояния
 *
 * # Arguments
 *
 * * `ctx_card`: Контекст для хранения данных библиотеки
 * * `data_group`: Указатель на область памяти, в которую будет записан хэш
 *
 * returns: [`i32`]
 *
 * # Examples
 *
 * ```C
 * Data *buf;
 * int32_t res = id_kta_version_info(&buf);
 * ```
 */
int32_t id_kta_version_info(const struct Data **output);
