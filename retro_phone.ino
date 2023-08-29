//  Телефонный аппарат с импульсным номеронабирателем на базе GSM/GPRS Shield A6
//  ============================================================================
                                             //
//  Определяем номера выводов:               //
    uint8_t pinBELL  = 2;                    // Вывод (выход) используемый для подключения силового ключа (для включения звонка вызова).
    uint8_t pinBEEP  = 3;                    // Вывод (выход) используемый для подключения излучателя звука (для вывода гудков в трубке).
    uint8_t pinHANG  = 4;                    // Вывод (вход)  используемый для подключения кнопки (NC) фиксирующей опускание телефонной трубки.
    uint8_t pinDIAL  = 5;                    // Вывод (вход)  используемый для подключения шунтирующего контакта (NO) номеронабирателя.
    uint8_t pinPULSE = 6;                    // Вывод (вход)  используемый для подключения тактирующего контакта (NC) номеронабирателя.
    uint8_t pinRX    = 7;                    // Вывод (вход)  используемый как линия RX (приём   ) программной шины UART (данные от GSM/GPRS Shield к  Arduino).
    uint8_t pinTX    = 8;                    // Вывод (выход) используемый как линия TX (передача) программной шины UART (данные к  GSM/GPRS Shield от Arduino).
                                             //
//  Определяем тип колокольного звонка:      //
    uint8_t modeBEEL = 0;                    // Колокольный звонок собран на соленоиде (электромагнитной катушке, собственном звонке телефона).
//  uint8_t modeBEEL = 1;                    // Колокольный звонок собран на электромоторе с бойком закреплённым к его ротору.
//  uint8_t modeBEEL = 2;                    // Колокольный звонок собран на сервоприводе  с бойком закреплённым к его качалке.
                                             //
//  Определяем алгоритм работы кнопки:       //
    bool    flgHANG  = 0;                    // 0-(NC) контакты кнопки замкнуты   при лежащей трубке.
//  bool    flgHANG  = 1;                    // 1-(NO) контакты кнопки разомкнуты при лежащей трубке.
                                             //
//  Объявляем переменные и функции:          //
    bool    flgPowerON = true;               // Определяем флаг подачи питания.
    uint8_t cntPULSE;                        // Объявляем переменную для подсчёта импульсов в последней набранной цифре (10 импульсов для цифры 0).
    uint8_t cntDigits;                       // Объявляем переменную для подсчёта набранных цифр номера.
    char    strNumber[12];                   // Объявляем строку для хранения номера вызываемого/вызывающего телефона (11 символов номера + символ конца строки).
    void    funcBELL(bool);                  // Объявляем функцию управления звонком (true - подаёт один колокольный звон, false - выключает колокольный звонок).
                                             //
//  Подключаем библиотеки:                   //
    #include "iarduino_GSM.h"                // Подключаем библиотеку iarduino_GSM для работы с GSM/GPRS Shield.
    #include <SoftwareSerial.h>              // Подключаем библиотеку SoftwareSerial для программной реализации шины UART.
    #include <Servo.h>                       // Подключаем библиотеку Servo для работы с сервоприводом (если колокольный звонок собран на сервоприводе).
                                             //
//  Создаём объекты:                         //
    iarduino_GSM   gsm(9);                   // Создаём объект gsm для работы с функциями и методами библиотеки iarduino_GSM, указав вывод PWR.
    SoftwareSerial softSerial(pinRX, pinTX); // Создаём объект softSerial указывая выводы платы Arduino используемые в качестве линий RX и TX программной шины UART.
    Servo          srv;                      // Создаём объект srv для работы с функциями и методами библиотеки Servo (если колокольный звонок собран на сервоприводе).
                                                                                                            //
void setup(){                                                                                               //
    Serial.begin(9600);                                                                                     // █ Инициируем связь с монитором последовательного порта на скорости 9600 бит/сек.
    if(modeBEEL==2){srv.attach(pinBELL);}                                                                   // Назначаем объекту srv управление сервоприводом подключённым к выводу pinBELL           (если колокольный звонок собран на сервоприводе).
    else{pinMode(pinBELL,OUTPUT); digitalWrite(pinBELL,LOW);}                                               // Переводим вывод pinBELL  в режим выхода и устанавливаем на нём уровень логического «0» (если колокольный звонок собран на соленоиде/электромагните/электромоторе).
    pinMode(pinBEEP,  OUTPUT); digitalWrite(pinBEEP,  LOW );                                                // Переводим вывод pinBEEP  в режим выхода и устанавливаем на нём уровень логического «0».
    pinMode(pinHANG,  INPUT ); digitalWrite(pinHANG,  HIGH);                                                // Переводим вывод pinHANG  в режим входа  и подтягиваем его к Vcc.
    pinMode(pinDIAL,  INPUT ); digitalWrite(pinDIAL,  HIGH);                                                // Переводим вывод pinDIAL  в режим входа  и подтягиваем его к Vcc.
    pinMode(pinPULSE, INPUT ); digitalWrite(pinPULSE, HIGH);                                                // Переводим вывод pinPULSE в режим входа  и подтягиваем его к Vcc.
                                                                                                            //
    funcBELL(false);                                                                                        // Отключаем колокольный звонок входящего вызова.
    gsm.begin(softSerial);                                                                                  // Инициируем работу GSM/GPRS Shield, указывая объект (или класс) для работы с её шиной UART.
                                                                                                            //
//  Ждём готовность GSM/GPRS Shield к работе:                                                               //
    while(gsm.status()!=GSM_OK){delay(1000);}                                                               // Ждём завершения регистрации модема в сети оператора связи.
                                                                                                            //
//  Переводим звук на гарнитуру (к ней подключена трубка):                                                  //
    gsm.SOUNDdevice(GSM_HEADSET);                                                                           // Для громкой связи нужно вместо GSM_HEADSET указать GSM_SPEAKER.
                                                                                                            //
//  Информируем о готовности модуля кратковременным включением колокольного звонка:                         //
    if(flgPowerON){                                                                                         //
//  Если функция setup() выполняется в первый раз:                                                          //
        uint32_t i = millis() + 1000;                                                                       // Определяем длительность звонка готовности модуля.
        while(i>millis()){ funcBELL(true); } funcBELL(false);                                               // Включаем и отключаем колокольный звонок.
        flgPowerON = false;                                                                                 // Сбрасываем флаг подачи питания.
    }                                                                                                       //
    Serial.println(F("Готов к работе!"));                                                                   // █ Можно добавить код выполняемый однократно после готовности аппарата при подаче питания.
}                                                                                                           //
                                                                                                            //
void loop (){                                                                                               //
    /******* СОВЕРШАЕМ ИСХОДЯЩИЙ ЗВОНОК *******/                                                            // Для исходящего звонка нужно поднять трубку и набрать номер.
    if(digitalRead(pinHANG)^flgHANG){                                                                       // Если на входе pinHANG установлена логическая «1» (трубка снята).
//  Если трубка снята:                                                                                      //
        delay(100);                                                                                         // Подавляем дребезг поднятия трубки.
//      Готовимся к набору номера:                                                                          // 
        cntDigits = 0;                                                                                      // Сбрасываем счетчик набранных цифр номера (номер ещё не набирался).
        strNumber[0]='\0';                                                                                  // Чистим строку набираемого номера.
        digitalWrite(pinBEEP, LOW);                                                                         // Отключаем тоновый сигнал в трубке телефона (если он был включён).
        Serial.println(F("Трубка снята, проверяем готовность к набору номера ..."));                        // █ Можно добавить код выполняемый однократно при поднятии трубки для набора номера, до проверки наличия связи с оператором.
//      Проверяем готовность GSM/GPRS Shield к работе:                                                      //
        if(gsm.status()!=GSM_OK){                                                                           //
//      Если модуль не готов к работе (например, ошибка регистрации в сети):                                //
            Serial.println(F("Перезагрузка модуля"));                                                       // █ Выводим сообщение о перезагрузке модуля.
//          Заново инициируем работу с модулем:                                                             //
            setup();                                                                                        //
        }                                                                                                   //
//      Информируем о готовности к набору номера:                                                           //
        digitalWrite(pinBEEP, HIGH);                                                                        // Включаем тоновый сигнал в трубке телефона (оповещая о готовности к набору номера).
        Serial.println(F("Можно набирать номер ..."));                                                      // █ Можно добавить код выполняемый однократно при поднятии трубки для набора номера, после проверки связи с оператором.
        while(digitalRead(pinHANG)^flgHANG){                                                                // Входим в цикл, который будет завершён опусканием трубки на телефон.
//      Цикл выполняется всё время, пока снята трубка:                                                      //
            if(!digitalRead(pinDIAL)){                                                                      // Если шунтирующая контактная группа номеронабирателя замкнулась (значат набор цифры), то ...
//          Если начинается набор очередной цифры номера:                                                   //
                delay(20);                                                                                  // Подавляем дребезг шунтирующей контактной группы номеронабирателя.
                digitalWrite(pinBEEP, LOW);                                                                 // Отключаем тоновый сигнал в трубке телефона (если он был включён).
                cntPULSE=0;                                                                                 // Сбрасываем счётчик поступивших импульсов от номеронабирателя.
                Serial.print(F("Набирается цифра ... "));                                                   // █ Можно добавить код выполняемый однократно перед набором каждой цифры номера
                while(!digitalRead(pinDIAL) && (digitalRead(pinHANG)^flgHANG)){                             // Если чтение импульсов набираемой цифры разрешено (шунтирующие контакты номеронабирателя замкнуты) и трубка снята, то ...
//              Цикл выполняется пока набирается очередная цифра номера:                                    //
                    if(digitalRead(pinPULSE)){                                                              // Если поступил тактирующий импульс (импульсная контактная группа номеронабирателя разомкнулась), то ...
//                      Фронт импульса:                                                                     //
                        digitalWrite(pinBEEP, HIGH);                                                        // Включаем тоновый сигнал в трубке телефона.
                        delay(5);                                                                           // Подавляем дребезг импульсной контактной группы номеронабирателя.
                        digitalWrite(pinBEEP, LOW);                                                         // Отключаем тоновый сигнал в трубке телефона.
                        while(digitalRead(pinPULSE) && (digitalRead(pinHANG)^flgHANG)){delay(5);}           // Ждём завершения тактирующего импульса (замыкания импульсной контактной группы номеронабирателя) или опускания трубки.
//                      Спад импульса:                                                                      //
                        delay(5);                                                                           // Подавляем дребезг импульскной контактной группы номеронабирателя.
                        cntPULSE++;                                                                         // Увеличиваем счётчик полученных импульсов.
                    }                                                                                       //
                }                                                                                           //
                delay(20);                                                                                  // Подавляем дребезг шунтирующей контактной группы номеронабирателя.
//              Очередная цифра номера набрана:                                                             //
                if(cntPULSE){                                                                               // Если от импульсной контактной группы номеронабирателя поступил хотя бы 1 импульс, то ...
//              Если цифра набрана корректно (во время набора поступил хотя бы один импульс)                //
                    if(cntPULSE>=10){cntPULSE=0;}                                                           // Если поступило 10 импульсов, значит набрана цифра 0.
                    strNumber[cntDigits]=cntPULSE+48;                                                       // Сохраняем код набранной цифры в строку с набираемым номером.
                    cntDigits++;                                                                            // Переходим к следующей цифре набираемого номера.
                    strNumber[cntDigits]='\0';                                                              // Сохраняем код конца строки.
                    Serial.println(cntPULSE);                                                               // █ Можно добавить код выполняемый однократно после набора каждой цифры номера.
                }                                                                                           //
//              Проверяем введённые цифры номера:                                                           //
                if(     cntDigits==11                                                                       // Если набрано 11 цифр  номера *(***)***-**-** - обычный номер.
                    || (cntDigits==7 && strncmp("8495100", strNumber, 8)==0)                                // Если набрано 7  цифр  номера 8(495)100       - точное время    (городской).
                    || (cntDigits==6 && strncmp("849501",  strNumber, 7)==0)                                // Если набрано 6  цифр  номера 8(495)01        - пожарная служба (городской).
                    || (cntDigits==6 && strncmp("849502",  strNumber, 7)==0)                                // Если набрано 6  цифр  номера 8(495)02        - полиция         (городской).
                    || (cntDigits==6 && strncmp("849503",  strNumber, 7)==0)                                // Если набрано 6  цифр  номера 8(495)03        - скорая помощь   (городской).
                    || (cntDigits==6 && strncmp("849504",  strNumber, 7)==0)                                // Если набрано 6  цифр  номера 8(495)04        - газовая служба  (городской).
                    || (cntDigits==3 && strncmp("101",     strNumber, 4)==0)                                // Если набрано 3  цифры номера 101             - пожарная служба.
                    || (cntDigits==3 && strncmp("102",     strNumber, 4)==0)                                // Если набрано 3  цифры номера 102             - полиция.
                    || (cntDigits==3 && strncmp("103",     strNumber, 4)==0)                                // Если набрано 3  цифры номера 103             - скорая помощь.
                    || (cntDigits==3 && strncmp("104",     strNumber, 4)==0)                                // Если набрано 3  цифры номера 104             - газовая служба.
                    || (cntDigits==3 && strncmp("112",     strNumber, 4)==0)                                // Если набрано 3  цифры номера 112             - экстренные оперативные службы.
                ){                                                                                          //
//              Если номер набран полностью, то инициируем вызов ...                                        //
                    if(gsm.CALLdial(strNumber)){                                                            // Инициируем исходящий голосовой вызов на номер указанный в строке strNumber.
//                  Если исходящий вызов инициирован, ждём завершения набора номера ...                     //
                        Serial.println((String) "Набор номера " + strNumber + " ...");                      // █ Можно добавить код выполняемый однократно при начале набора номера.
                        while(gsm.CALLstatus()==GSM_CALL_OUT_DIAL && (digitalRead(pinHANG)^flgHANG)){}      // Цикл выполняется пока установлено состояние вызова "набирается номер" и снята трубка.
                        while(gsm.CALLstatus()==GSM_CALL_OUT_DIAL && (digitalRead(pinHANG)^flgHANG)){}      // Повторяем цикл на случай кратковременного изменения статуса вызова.
                        while(gsm.CALLstatus()==GSM_CALL_OUT_DIAL && (digitalRead(pinHANG)^flgHANG)){}      // Повторяем цикл на случай кратковременного изменения статуса вызова.
                        if(gsm.CALLstatus()==GSM_OK){                                                       //
//                      Если произошёл обрыв связи с оператором:                                            //
                            Serial.println(F("произошёл обрыв связи с оператором."));                       // █ Можно добавить код выполняемый однократно при обрыве связи с оператором.
                        }                                                                                   //
                        if(gsm.CALLstatus()==GSM_CALL_OUT_BEEP){                                            // Если установилось состояние вызова "дозвон", то ...
//                      Если начался дозвон, то ждём пока вызываемый абонент не ответит ...                 //
                            Serial.println(F("Ожидание ответа ..."));                                       // █ Можно добавить код выполняемый однократно при поступлении гудков у вызываемого абонента.
                            while(gsm.CALLstatus()==GSM_CALL_OUT_BEEP && (digitalRead(pinHANG)^flgHANG)){}  // Цикл выполняется пока установлено состояние вызова "дозвон" и снята трубка.
                            delay(500);                                                                     // Даём время для установки состояния вызова - "соединён".
                        }                                                                                   //
                        if(gsm.CALLstatus()==GSM_CALL_ACTIVE){                                              // Если установилось состояние вызова "соединён", то ...
//                      Если установлено активное голосовое соединение ...                                  //
                            Serial.println(F("Исходящее голосовое соединение установлено."));               // █ Можно добавить код выполняемый однократно при установлении активного голосового соединения.
                            while(gsm.CALLstatus()==GSM_CALL_ACTIVE && (digitalRead(pinHANG)^flgHANG)){}    // Цикл выполняется пока установлено активное голосовое соединение и снята трубка.
//                          Если голосовое соединение разорвано или его требуется разорвать ...             // 
                        }                                                                                   //
                        Serial.println(F("Разговор завершён."));                                            // █ Можно добавить код выполняемый однократно в момент завершения разговора.
                    }                                                                                       //
//                  Разрываем голосовое соединение, если разговор завершён опусканием трубки:               //
                    gsm.CALLend();                                                                          // Разъединяем голосовое соединение.
//                  Выводим короткие звуковые сигналы в трубку телефона...                                  //
                    while(digitalRead(pinHANG)^flgHANG){                                                    // Цикл выполняется пока снята трубка.
                        if(millis()%1000<500){digitalWrite(pinBEEP, HIGH);}                                 // Выводим   тоновый сигнал в трубке телефона в течении первых 500 мс каждых  1000 мс.
                        else                 {digitalWrite(pinBEEP, LOW );}                                 // Отключаем тоновый сигнал в трубке телефона в течении остального времени из 1000 мс.
                    }                         digitalWrite(pinBEEP, LOW );                                  // Отключаем тоновый сигнал в трубке телефона.
                }                                                                                           //
            }                                                                                               //
            gsm.CALLend();                                                                                  // Разъединяем голосовое соединение, если нам позвонили пока поднята трубка (до или в момент набора номера).
        }                                                                                                   //
        Serial.println(F("Трубка опущена на аппарат."));                                                    // █ Можно добавить код выполняемый однократно в момент опускания трубки на аппарат.
    }else{                                                                                                  //
    /******* ПРИНИМАЕМ ВХОДЯЩИЙ ЗВОНОК *******/                                                             // Для приёма входящих звонков трубка должна быть опущена.
//  Если трубка лежит на телефоне:                                                                          //
        delay(100);                                                                                         // Подавляем дребезг опускания трубки.
        digitalWrite(pinBEEP, LOW);                                                                         // Отключаем тоновый сигнал в трубке телефона (если он был включён).
        Serial.println(F("Трубка лежит на аппарате, режим ожидания звонка ..."));                           // █ Можно добавить код выполняемый однократно в момент перехода в режим ожидания входящего звонка
        while(!digitalRead(pinHANG)^flgHANG){                                                               // Входим в цикл, который будет завершён поднятием трубки с телефона.
//      Цикл выполняется всё время, пока трубка не поднята:                                                 //
            if(gsm.CALLavailable(strNumber)){                                                               // Функция CALLavailable() возвращает true если есть входящий дозванивающийся вызов, номер вызывающего абонента сохраняется в строку strNumber.
//          Если есть входящий вызов в режиме дозвона, то ждём ответа поднятием трубки ...                  //
                Serial.println((String)"Входящий вызов "+strNumber+", ждём поднятия трубки ...");           // █ Можно добавить код выполняемый однократно в момент поступления входящего звонка
                while(gsm.CALLavailable() && !(digitalRead(pinHANG)^flgHANG)){                              // Цикл выполняется пока есть входящий вызов в режиме дозвона и трубка не поднята.
//              Информируем колокольными звонками о наличии входящего вызова:                               //
                    while(millis()%4000<2000){funcBELL(true);}                                              // Включаем колокольный звонок в течении первых 2000 мс каждых 4000 мс.
                                              funcBELL(false);                                              // Отключаем колокольный звонок в течении остального времени.
                }                                                                                           //
                delay(100);                                                                                 // Подавляем дребезг поднятия трубки.
//              Проверяем почему был завершён цикл ожидания ответа ...                                      //
                if(digitalRead(pinHANG)^flgHANG){                                                           // Если трубка снята.
//              Если цикл завершён по причине поднятия трубки:                                              // 
                    Serial.println(F("Трубка снята, отвечаем на звонок"));                                  // █ Можно добавить код выполняемый однократно в момент поднятия трубки для ответа на входящий звонок.
                    if(gsm.CALLavailable()){                                                                // Функция CALLavailable() возвращает true если есть входящий дозванивающийся вызов.
//                  Если вызывающий абонент всё ещё ждёт ответа (поднятия трубки) ...                       //
                        gsm.CALLup();                                                                       // Отвечаем на вызов.
//                      Ждём пока состояние вызова "дозвон" не сменится ...                                 //
                        while(gsm.CALLstatus()==GSM_CALL_IN_BEEP){;}                                        // Функция CALLstatus() возвращает статус текущего голосового вызова, значение GSM_CALL_IN_BEEP указывает на наличие входящего дозванивающегося вызова.
                        if(gsm.CALLstatus()==GSM_CALL_ACTIVE){                                              // Функция CALLstatus() возвращает статус текущего голосового вызова, значение GSM_CALL_ACTIVE указывает на наличие активного голосового соединения.
//                      Если установлено активное голосовое соединение ...                                  //
                            Serial.println(F("Входящее голосовое соединение установлено."));                // █ Можно добавить код выполняемый однократно при установлении активного голосового соединения.
                            while(gsm.CALLstatus()==GSM_CALL_ACTIVE && (digitalRead(pinHANG)^flgHANG)){}    // Цикл выполняется пока установлено активное голосовое соединение и снята трубка.
                        }                                                                                   //
//                      Если голосовое соединение разорвано или требуется разорвать ...                     // 
                        Serial.println(F("Разговор завершён."));                                            // █ Можно добавить код выполняемый однократно в момент завершения разговора.
//                      Разрываем голосовое соединение, если разговор завершён опусканием трубки:           //
                        gsm.CALLend();                                                                      // Разъединяем голосовое соединение, это требуется если мы инициировали разрыв соединения опусканием трубки.
                    }                                                                                       //
//                  Выводим короткие звуковые сигналы в трубку телефона...                                  //
                    while(digitalRead(pinHANG)^flgHANG){                                                    // Цикл выполняется пока снята трубка.
                        if(millis()%1000<500){digitalWrite(pinBEEP, HIGH);}                                 // Выводим   тоновый сигнал в трубке телефона в течении первых 500 мс каждых  1000 мс.
                        else                 {digitalWrite(pinBEEP, LOW );}                                 // Отключаем тоновый сигнал в трубке телефона в течении остального времени из 1000 мс.
                    }                         digitalWrite(pinBEEP, LOW );                                  // Отключаем тоновый сигнал в трубке телефона.
                }else{                                                                                      //
//              Если цикл завершён по причине сброса вызова:                                                // 
                    Serial.println(F("Вызов завершён по причине сброса вызова"));                           // █ Можно добавить код выполняемый однократно в момент сброва вызова.
                }                                                                                           //
            }else{                                                                                          //
//          Если входящих вызовов в режиме дозвона нет:                                                     //
                if(gsm.status()!=GSM_OK){                                                                   //
//              Если модуль не готов к работе (например, ошибка регистрации в сети):                        //
                    Serial.println(F("Перезагрузка модуля"));                                               // █ Выводим сообщение о перезагрузке модуля.
//                  Заново инициируем работу с модулем:                                                     //
                    setup();                                                                                //
                }                                                                                           //
            }                                                                                               //
        }                                                                                                   //
    }                                                                                                       //
}                                                                                                           //
                                                                                                            //
//  Функция управления колокольным звонком:                                                                 // В зависимости от параметра (f) функция либо отключает колокольный звонок, либо подаёт один колокольный звон входящего вызова.
void funcBELL(bool f){                                                                                      // В данной функции можно регулировать тональность колокольного звонка, меняя задержку delay().
    if(modeBEEL==0){                                                                                        //
//  Если колокольный звонок собран на соленоиде (электромагнитной катушке):                                 //
        if(f){digitalWrite(pinBELL, HIGH); delay(20);                                                       // Если установлен флаг f, то: - подаём высокий уровень на выход pinBELL (силовой ключ замкнётся  , через катушку потечёт ток и боёк ударит о колокол),    ждём 20 мс.
              digitalWrite(pinBELL, LOW ); delay(20);                                                       //                             - подаём низкий  уровень на выход pinBELL (силовой ключ разомкнётся, катушка будет обесточена и боёк удалится от колокола), ждём 20 мс.
        }else{digitalWrite(pinBELL, LOW );}                                                                 // Если сброшен флаг f, то     - подаём низкий  уровень на выход pinBELL (силовой ключ разомкнётся, катушка будет обесточена и боёк удалится от колокола).
    }else if(modeBEEL==1){                                                                                  //
//  Если колокольный звонок собран на электромоторе:                                                        //
        if(f){digitalWrite(pinBELL, HIGH);}                                                                 // Если установлен флаг f, то  - подаём высокий уровень на выход pinBELL (силовой ключ замкнётся  , электромотор включится и боёк на его роторе начнёт бить по колоколу).
        else {digitalWrite(pinBELL, LOW );}                                                                 // Если сброшен флаг f, то     - подаём низкий  уровень на выход pinBELL (силовой ключ разомкнётся, электромотор отключится и боёк перестанет бить по колоколу).
    }else if(modeBEEL==2){                                                                                  //
//  Если колокольный звонок собран на сервоприводе:                                                         //
        if(f){srv.write(50); delay(20);                                                                     // Если установлен флаг f, то: - поворачиваем сервопривод на угол при котором боёк закреплённый к его качалке ударит о колокол,     ждём 20 мс.
              srv.write(60); delay(20);                                                                     //                             - поворачиваем сервопривод на угол при котором боёк закреплённый к его качалке удалится от колокола, ждём 20 мс.
        }else{srv.write(60);}                                                                               // Если сброшен флаг f, то     - поворачиваем сервопривод на угол при котором боёк закреплённый к его качалке удалится от колокола.
    }                                                                                                       //                               Вместо углов 50° и 60° необходимо указать Ваши углы (значение подбирается экспериментально).
}
