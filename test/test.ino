/**
 *  E2E tests for InfluxDBClient.
 *  For compiling in VSCode add path to workspace ("${workspaceFolder}\\**")
 * 
 */

#define INFLUXDB_CLIENT_TESTING
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#if defined(ESP32)
String chipId = String((unsigned long)ESP.getEfuseMac());
String deviceName = "ESP32";
#elif defined(ESP8266)
String chipId = String(ESP.getChipId());
String deviceName = "ESP8266";
#endif

#define INFLUXDB_CLIENT_TESTING_URL "http://192.168.88.142:999"
#define INFLUXDB_CLIENT_TESTING_ORG "my-org"
#define INFLUXDB_CLIENT_TESTING_BUC "my-bucket"
#define INFLUXDB_CLIENT_TESTING_DB "my-db"
#define INFLUXDB_CLIENT_TESTING_TOK "1234567890"
#define INFLUXDB_CLIENT_TESTING_SSID "SSID"
#define INFLUXDB_CLIENT_TESTING_PASS "password"
#define INFLUXDB_CLIENT_TESTING_BAD_URL "http://127.0.0.1:999"

#include "customSettings.h"

#include "TestSupport.h"
#include <core_version.h>

void setup() {
    Serial.begin(115200);

    //Serial.setDebugOutput(true);
    randomSeed(123);

    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(true);

    Serial.println();

    initInet();

    Serial.printf("Using server: %s\n", INFLUXDB_CLIENT_TESTING_URL);
}

class Test {
public:
    static void run();
private:
    static void testOptions();
    static void testEcaping();
    static void testPoint();
    static void testFluxTypes();
    static void testFluxParserEmpty();
    static void testFluxParserSingleTable();
    static void testFluxParserNilValue();
    static void testFluxParserMultiTables(bool chunked);
    static void testFluxParserErrorDiffentColumnsNum();
    static void testFluxParserFluxError();
    static void testFluxParserInvalidDatatype();
    static void testFluxParserMissingDatatype();
    static void testFluxParserErrorInRow();
    static void testBasicFunction();
    static void testInit();
    static void testV1();
    static void testUserAgent();
    static void testFailedWrites();
    static void testTimestamp();
    static void testHTTPReadTimeout();
    static void testRetryOnFailedConnection();
    static void testBufferOverwriteBatchsize1();
    static void testBufferOverwriteBatchsize5();
    static void testServerTempDownBatchsize5();
    static void testRetriesOnServerOverload();
    static void testRetryInterval();
};

void loop() {
    Serial.printf("RAM %d\n", ESP.getFreeHeap());
    Test::run();
    Serial.printf("Test %s\n", failures ? "FAILED" : "SUCCEEDED");
    Serial.printf("RAM %d\n", ESP.getFreeHeap());
    while(1) delay(1000);
}

void Test::run() {
    // Basic tests
    testOptions();
    testEcaping();
    testPoint();
    testFluxTypes();
    testFluxParserEmpty();
    testFluxParserSingleTable();
    testFluxParserNilValue();
    testFluxParserMultiTables(false);
    testFluxParserMultiTables(true);
    testFluxParserErrorDiffentColumnsNum();
    testFluxParserFluxError();
    testFluxParserInvalidDatatype();
    testFluxParserMissingDatatype();
    testFluxParserErrorInRow();
    testBasicFunction();
    testInit();
    testV1();
    testUserAgent();
    testHTTPReadTimeout();
    // Advanced tests
    testFailedWrites();
    testTimestamp();
    testRetryOnFailedConnection();
    testBufferOverwriteBatchsize1();
    testBufferOverwriteBatchsize5();
    testServerTempDownBatchsize5();
    testRetriesOnServerOverload();
    testRetryInterval();
}

void Test::testOptions() {
        TEST_INIT("testOptions");
        WriteOptions defWO;
        TEST_ASSERT(defWO._writePrecision == WritePrecision::NoTime);
        TEST_ASSERT(defWO._batchSize == 1);
        TEST_ASSERT(defWO._bufferSize == 5);
        TEST_ASSERT(defWO._flushInterval == 60);
        TEST_ASSERT(defWO._retryInterval == 5);
        TEST_ASSERT(defWO._maxRetryInterval == 300);
        TEST_ASSERT(defWO._maxRetryAttempts == 3);

        defWO = WriteOptions().writePrecision(WritePrecision::NS).batchSize(10).bufferSize(20).flushInterval(120).retryInterval(1).maxRetryInterval(20).maxRetryAttempts(5);
        TEST_ASSERT(defWO._writePrecision == WritePrecision::NS);
        TEST_ASSERT(defWO._batchSize == 10);
        TEST_ASSERT(defWO._bufferSize == 20);
        TEST_ASSERT(defWO._flushInterval == 120);
        TEST_ASSERT(defWO._retryInterval == 1);
        TEST_ASSERT(defWO._maxRetryInterval == 20);
        TEST_ASSERT(defWO._maxRetryAttempts == 5);

        HTTPOptions defHO;
        TEST_ASSERT(!defHO._connectionReuse);
        TEST_ASSERT(defHO._httpReadTimeout == 5000);

        defHO = HTTPOptions().connectionReuse(true).httpReadTimeout(20000);
        TEST_ASSERT(defHO._connectionReuse);
        TEST_ASSERT(defHO._httpReadTimeout == 20000);

        InfluxDBClient c;
        TEST_ASSERT(c._writeOptions._writePrecision == WritePrecision::NoTime);
        TEST_ASSERT(c._writeOptions._batchSize == 1);
        TEST_ASSERT(c._writeOptions._bufferSize == 5);
        TEST_ASSERT(c._writeOptions._flushInterval == 60);
        TEST_ASSERT(c._writeOptions._retryInterval == 5);
        TEST_ASSERT(c._writeOptions._maxRetryAttempts == 3);
        TEST_ASSERT(c._writeOptions._maxRetryInterval == 300);
        TEST_ASSERT(!c._httpOptions._connectionReuse);
        TEST_ASSERT(c._httpOptions._httpReadTimeout == 5000);

        c.setWriteOptions(defWO);
        TEST_ASSERT(c._writeOptions._writePrecision == WritePrecision::NS);
        TEST_ASSERT(c._writeOptions._batchSize == 10);
        TEST_ASSERT(c._writeOptions._bufferSize == 20);
        TEST_ASSERT(c._writeOptions._flushInterval == 120);
        TEST_ASSERT(c._writeOptions._retryInterval == 1);
        TEST_ASSERT(c._writeOptions._maxRetryAttempts == 5);
        TEST_ASSERT(c._writeOptions._maxRetryInterval == 20);

        c.setHTTPOptions(defHO);
        TEST_ASSERT(c._httpOptions._connectionReuse);
        TEST_ASSERT(c._httpOptions._httpReadTimeout == 20000);

        c.setWriteOptions(WritePrecision::MS, 15, 14, 70, false);
        TEST_ASSERT(c._writeOptions._writePrecision == WritePrecision::MS);
        TEST_ASSERT(c._writeOptions._batchSize == 15);
        TEST_ASSERTM(c._writeOptions._bufferSize == 30, String(c._writeOptions._bufferSize));
        TEST_ASSERT(c._writeOptions._flushInterval == 70);
        TEST_ASSERT(!c._httpOptions._connectionReuse);
        TEST_ASSERT(c._httpOptions._httpReadTimeout == 20000);

        TEST_END();
    }


void Test::testEcaping() {
    TEST_INIT("testEcaping");

    Point p("t\re=s\nt\t_t e\"s,t");
    p.addTag("ta=g","val=ue");
    p.addTag("ta\tg","val\tue");
    p.addTag("ta\rg","val\rue");
    p.addTag("ta\ng","val\nue");
    p.addTag("ta g","valu e");
    p.addTag("ta,g","valu,e");
    p.addTag("tag","value");
    p.addTag("ta\"g","val\"ue");
    p.addField("fie=ld", "val=ue");
    p.addField("fie\tld", "val\tue");
    p.addField("fie\rld", "val\rue");
    p.addField("fie\nld", "val\nue");
    p.addField("fie ld", "val ue");
    p.addField("fie,ld", "val,ue");
    p.addField("fie\"ld", "val\"ue");
    
    String line = p.toLineProtocol();
    TEST_ASSERTM(line == "t\\\re=s\\\nt\\\t_t\\ e\"s\\,t,ta\\=g=val\\=ue,ta\\\tg=val\\\tue,ta\\\rg=val\\\rue,ta\\\ng=val\\\nue,ta\\ g=valu\\ e,ta\\,g=valu\\,e,tag=value,ta\"g=val\"ue fie\\=ld=\"val=ue\",fie\\\tld=\"val\tue\",fie\\\rld=\"val\rue\",fie\\\nld=\"val\nue\",fie\\ ld=\"val ue\",fie\\,ld=\"val,ue\",fie\"ld=\"val\\\"ue\"", line);//
    TEST_END();
}


void Test::testPoint() {
    TEST_INIT("testPoint");

    Point p("test");
    TEST_ASSERT(!p.hasTags());
    TEST_ASSERT(!p.hasFields());
    p.addTag("tag1", "tagvalue");
    TEST_ASSERT(p.hasTags());
    TEST_ASSERT(!p.hasFields());
    p.addField("fieldInt", -23);
    TEST_ASSERT(p.hasFields());
    p.addField("fieldBool", true);
    p.addField("fieldFloat1", 1.123f);
    p.addField("fieldFloat2", 1.12345f, 5);
    p.addField("fieldDouble1", 1.123);
    p.addField("fieldDouble2", 1.12345, 5);
    p.addField("fieldChar", 'A');
    p.addField("fieldUChar", (unsigned char)1);
    p.addField("fieldUInt", 23u);
    p.addField("fieldLong", 123456l);
    p.addField("fieldULong", 123456ul);
    p.addField("fieldString", "text test");
    String line = p.toLineProtocol();
    String testLine = "test,tag1=tagvalue fieldInt=-23i,fieldBool=true,fieldFloat1=1.12,fieldFloat2=1.12345,fieldDouble1=1.12,fieldDouble2=1.12345,fieldChar=\"A\",fieldUChar=1i,fieldUInt=23i,fieldLong=123456i,fieldULong=123456i,fieldString=\"text test\"";
    TEST_ASSERTM(line == testLine, line);

    p.clearFields();
    p.clearTags();

    //line protocol without tags
    p.addField("f", 1);
    line = p.toLineProtocol();
    testLine = "test f=1i";
    TEST_ASSERTM(line == testLine, line);

    TEST_ASSERT(!p.hasTime());
    time_t now = time(nullptr);
    String snow(now);
    p.setTime(now);
    String testLineTime = testLine + " " + snow;
    line = p.toLineProtocol();
    TEST_ASSERTM(line == testLineTime, line);
    
    unsigned long long ts = now*1000000000LL+123456789;
    p.setTime(ts);
    testLineTime = testLine + " " + snow + "123456789";
    line = p.toLineProtocol();
    TEST_ASSERTM(line == testLineTime, line);

    now += 10;
    snow = now;
    p.setTime(snow);
    testLineTime = testLine + " " + snow;
    line = p.toLineProtocol();
    TEST_ASSERTM(line == testLineTime, line);

    p.setTime(WritePrecision::S);
    line = p.toLineProtocol();
    int partsCount;
    String *parts = getParts(line, ' ', partsCount);
    TEST_ASSERTM(partsCount == 3, String("3 != ") + partsCount);
    TEST_ASSERT(parts[2].length() == snow.length());
    delete[] parts;

    p.setTime(WritePrecision::MS);
    TEST_ASSERT(p.hasTime());
    line = p.toLineProtocol();
    parts = getParts(line, ' ', partsCount);
    TEST_ASSERT(partsCount == 3);
    TEST_ASSERT(parts[2].length() == snow.length() + 3);
    delete[] parts;

    p.setTime(WritePrecision::US);
    line = p.toLineProtocol();
    parts = getParts(line, ' ', partsCount);
    TEST_ASSERT(partsCount == 3);
    TEST_ASSERT(parts[2].length() == snow.length() + 6);
    delete[] parts;

    p.setTime(WritePrecision::NS);
    line = p.toLineProtocol();
    parts = getParts(line, ' ', partsCount);
    TEST_ASSERT(partsCount == 3);
    TEST_ASSERT(parts[2].length() == snow.length() + 9);
    delete[] parts;

    p.clearFields();
    TEST_ASSERT(!p.hasFields());
    p.clearTags();
    TEST_ASSERT(!p.hasFields());
    p.setTime("");
    TEST_ASSERT(!p.hasTime());

    p.addField("nan", (float)NAN);
    TEST_ASSERT(!p.hasFields());
    p.addField("nan", (double)NAN);
    TEST_ASSERT(!p.hasFields());

    TEST_END();
}

void Test::testBasicFunction() {
    TEST_INIT("testBasicFunction");

    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_BAD_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    TEST_ASSERT(!client.isBufferFull());
    TEST_ASSERT(client.isBufferEmpty());
    TEST_ASSERT(!client.validateConnection());
    for (int i = 0; i < 5; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        TEST_ASSERT(!client.writePoint(*p));
        delete p;
    }
    TEST_ASSERT(client.isBufferFull());
    TEST_ASSERT(!client.isBufferEmpty());
    client.setConnectionParams(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    TEST_ASSERT(client.isBufferFull());
    TEST_ASSERT(!client.isBufferEmpty());
    client.resetBuffer();
    
    TEST_ASSERT(waitServer(client, true));
    for (int i = 0; i < 5; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        TEST_ASSERT(client.writePoint(*p));
        delete p;
    }
    TEST_ASSERT(client.isBufferEmpty());
    String query = "select";
    FluxQueryResult q = client.query(query);
    int count = countLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERTM( count == 5, String(count) + " vs 5");  //5 points

    // test precision
    for (int i = (int)WritePrecision::NoTime; i <= (int)WritePrecision::NS; i++) {
        client.setWriteOptions((WritePrecision)i, 1);
        Point *p = createPoint("test1");
        p->addField("index", i);
        TEST_ASSERTM(client.writePoint(*p), String("i=") + i);
        delete p;
    }

    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
}

void Test::testInit() {
    TEST_INIT("testInit");
    {
        InfluxDBClient client;
        TEST_ASSERT(!client.validateConnection());
        TEST_ASSERT(client.getLastStatusCode() == 0);
        TEST_ASSERT(client.getLastErrorMessage() == "Unconfigured instance");

    }
    {
        InfluxDBClient client;
        String rec = "a,a=1 a=3";
        TEST_ASSERT(!client.writeRecord(rec));
        TEST_ASSERT(client.getLastStatusCode() == 0);
        TEST_ASSERT(client.getLastErrorMessage() == "Unconfigured instance");
    }
    {
        InfluxDBClient client;
        String query = "select";
        FluxQueryResult q = client.query(query);
        TEST_ASSERT(!q.next());
        TEST_ASSERT(q.getError() == "Unconfigured instance");
        TEST_ASSERT(client.getLastStatusCode() == 0);
        TEST_ASSERT(client.getLastErrorMessage() == "Unconfigured instance");

        client.setConnectionParams(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
        String rec = "a,a=1 a=3";
        TEST_ASSERT(client.writeRecord(rec));
        q = client.query(query);
        TEST_ASSERT(countLines(q) == 1);  
        TEST_ASSERTM(q.getError()=="", q.getError());
    }

    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
}

#define STRHELPER(x) #x
#define STR(x) STRHELPER(x) // stringifier

#if defined(ESP8266)
# define INFLUXDB_CLIENT_PLATFORM "ESP8266"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  STR(ARDUINO_ESP8266_GIT_DESC)
#elif defined(ESP32)
# define INFLUXDB_CLIENT_PLATFORM "ESP32"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  STR(ARDUINO_ESP32_GIT_DESC)
#endif


void Test::testUserAgent() {
    TEST_INIT("testUserAgent");

    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    waitServer(client, true);
    TEST_ASSERT(client.validateConnection());
    String url = INFLUXDB_CLIENT_TESTING_URL "/test/user-agent";
    HTTPClient http;
    TEST_ASSERT(http.begin(url));
    TEST_ASSERT(http.GET() == 200);
    String agent = "influxdb-client-arduino/" INFLUXDB_CLIENT_VERSION " (" INFLUXDB_CLIENT_PLATFORM " " INFLUXDB_CLIENT_PLATFORM_VERSION ")";
    String data = http.getString();
    TEST_ASSERTM(data == agent, data);
    http.end();
    TEST_END();
}

void Test::testHTTPReadTimeout() {
    TEST_INIT("testHTTPReadTimeout");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    waitServer(client, true);
    TEST_ASSERT(client.validateConnection());
    //set server delay for 6s (client has default timeout 5s)
    String rec = "a,direction=timeout,timeout=6 a=1";
    TEST_ASSERT(client.writeRecord(rec));
    rec = "a,tag=a, a=1i";
    TEST_ASSERT(client.writeRecord(rec));

    String query = "select";
    FluxQueryResult q = client.query(query);
    // should timeout
    TEST_ASSERT(!q.next());
    TEST_ASSERTM(q.getError() == "read Timeout", q.getError());
    q.close();
    rec = "a,direction=timeout,timeout=4 a=1";
    TEST_ASSERT(client.writeRecord(rec));
    q = client.query(query);
    // should be ok
    TEST_ASSERT(q.next());
    TEST_ASSERT(!q.next());
    TEST_ASSERTM(q.getError() == "", q.getError());
    q.close();
    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
}

void Test::testRetryOnFailedConnection() {
    TEST_INIT("testRetryOnFailedConnection");

    InfluxDBClient clientOk(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    clientOk.setWriteOptions(WriteOptions().batchSize(1).bufferSize(5));
    clientOk.setHTTPOptions(HTTPOptions().httpReadTimeout(500));

    Serial.println("Stop server!");
    waitServer(clientOk, false);
    TEST_ASSERT(!clientOk.validateConnection());
    Point *p = createPoint("test1");
    TEST_ASSERT(!clientOk.writePoint(*p));
    delete p;
    p = createPoint("test1");
    TEST_ASSERT(!clientOk.writePoint(*p));
    delete p;

    Serial.println("Start server!");
    waitServer(clientOk, true);
    clientOk.setHTTPOptions(HTTPOptions().httpReadTimeout(5000));
    TEST_ASSERT(clientOk.validateConnection());
    p = createPoint("test1");
    TEST_ASSERT(clientOk.writePoint(*p));
    delete p;
    TEST_ASSERT(clientOk.isBufferEmpty());
    String query = "select";
    FluxQueryResult q = clientOk.query(query);
    TEST_ASSERT(countLines(q) == 3);

    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
}

void Test::testBufferOverwriteBatchsize1() {
    TEST_INIT("testBufferOverwriteBatchsize1");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_BAD_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    client.setWriteOptions(WriteOptions().batchSize(1).bufferSize(5));
    client.setHTTPOptions(HTTPOptions().httpReadTimeout(500));

    TEST_ASSERT(!client.validateConnection());
    for (int i = 0; i < 12; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        TEST_ASSERT(!client.writePoint(*p));
        delete p;
    }
    TEST_ASSERT(client.isBufferFull());
    TEST_ASSERTM(client._writeBuffer[0]->buffer[0].indexOf("index=10i") > 0, client._writeBuffer[0]->buffer[0]);

    client._serverUrl = INFLUXDB_CLIENT_TESTING_URL;
    client.setUrls();
    waitServer(client, true);
    client.setHTTPOptions(HTTPOptions().httpReadTimeout(5000));
    Point *p = createPoint("test1");
    p->addField("index", 12);
    TEST_ASSERT(client.writePoint(*p));
    TEST_ASSERT(client.isBufferEmpty());

    String query = "select";
    FluxQueryResult q = client.query(query);
    std::vector<String> lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERTM(lines.size() == 5, String("5 != " + lines.size()));  //5 points
    TEST_ASSERTM(lines[0].indexOf(",8") > 0, lines[0]);
    TEST_ASSERTM(lines[1].indexOf(",9") > 0, lines[1]);
    TEST_ASSERTM(lines[2].indexOf(",10") > 0, lines[2]);
    TEST_ASSERTM(lines[3].indexOf(",11") > 0, lines[3]);
    TEST_ASSERTM(lines[4].indexOf(",12") > 0, lines[4]);

    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
}

void Test::testBufferOverwriteBatchsize5() {
    TEST_INIT("testBufferOverwriteBatchsize5");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_BAD_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    client.setWriteOptions(WriteOptions().batchSize(5).bufferSize(20));
    client.setHTTPOptions(HTTPOptions().httpReadTimeout(500));

    TEST_ASSERT(!client.validateConnection());
    for (int i = 0; i < 39; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        //will succeed only first batchsize-1 points
        TEST_ASSERTM(client.writePoint(*p) == (i < 4), String("i=") + i);
        delete p;
    }
    TEST_ASSERT(client.isBufferFull());
    TEST_ASSERTM(client._writeBuffer[0]->buffer[0].indexOf("index=20i") > 0, client._writeBuffer[0]->buffer[0]);

    client._serverUrl = INFLUXDB_CLIENT_TESTING_URL;
    client.setUrls();

    waitServer(client, true);
    client.setHTTPOptions(HTTPOptions().httpReadTimeout(5000));
    Point *p = createPoint("test1");
    p->addField("index", 39);
    TEST_ASSERT(client.writePoint(*p));
    TEST_ASSERT(client.isBufferEmpty());
    //flushing of empty buffer is ok
    TEST_ASSERT(client.flushBuffer());

    String query = "select";
    FluxQueryResult q = client.query(query);
    std::vector<String> lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERTM(lines.size() == 20,String(lines.size()));  //20 points (4 batches)
    TEST_ASSERTM(lines[0].indexOf(",20") > 0,lines[0]);
    TEST_ASSERTM(lines[1].indexOf(",21") > 0,lines[1]);
    TEST_ASSERTM(lines[2].indexOf(",22") > 0,lines[2]);
    TEST_ASSERTM(lines[3].indexOf(",23") > 0,lines[3]);
    TEST_ASSERTM(lines[4].indexOf(",24") > 0,lines[4]);
    TEST_ASSERTM(lines[19].indexOf(",39") > 0,lines[9]);
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
    // buffer has been emptied, now writes should go according batch size
    for (int i = 0; i < 4; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        TEST_ASSERT(client.writePoint(*p));
        delete p;
    }
    TEST_ASSERT(!client.isBufferEmpty());
    q = client.query(query);
    TEST_ASSERT(countLines(q) == 0);
    TEST_ASSERTM(q.getError()=="", q.getError());

    p = createPoint("test1");
    p->addField("index", 4);
    TEST_ASSERT(client.writePoint(*p));
    TEST_ASSERT(client.isBufferEmpty());
    
    q = client.query(query);
    lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERT(lines.size() == 5);  
    TEST_ASSERT(lines[0].indexOf(",0") > 0);
    TEST_ASSERT(lines[1].indexOf(",1") > 0);
    TEST_ASSERT(lines[2].indexOf(",2") > 0);
    TEST_ASSERT(lines[3].indexOf(",3") > 0);
    TEST_ASSERT(lines[4].indexOf(",4") > 0);

    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
}

void Test::testServerTempDownBatchsize5() {
    TEST_INIT("testServerTempDownBatchsize5");
    InfluxDBClient client;
    client.setConnectionParams(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    client.setWriteOptions(WriteOptions().batchSize(5).bufferSize(20).flushInterval(60));
    client.setHTTPOptions(HTTPOptions().connectionReuse(true));
    
    waitServer(client, true);
    TEST_ASSERT(client.validateConnection());
    for (int i = 0; i < 15; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        TEST_ASSERTM(client.writePoint(*p), String("i=") + i);
        delete p;
    }
    TEST_ASSERT(client.isBufferEmpty());
    String query = "select";
    FluxQueryResult q = client.query(query);
    TEST_ASSERT(countLines(q) == 15);  
    TEST_ASSERTM(q.getError()=="", q.getError());
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);

    Serial.println("Stop server");
    waitServer(client, false);
    client.setHTTPOptions(HTTPOptions().httpReadTimeout(500));
    for (int i = 0; i < 14; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        //will succeed only first batchsize-1 points
        TEST_ASSERTM(client.writePoint(*p) == (i < 4), String("i=") + i);
        delete p;
    }
    TEST_ASSERT(!client.isBufferEmpty());

    Serial.println("Start server");
    ;
    waitServer(client, true);
    client.setHTTPOptions(HTTPOptions().httpReadTimeout(5000));
    Point *p = createPoint("test1");
    p->addField("index", 14);
    TEST_ASSERT(client.writePoint(*p));
    TEST_ASSERT(client.isBufferEmpty());
    q = client.query(query);
    TEST_ASSERT(countLines(q) == 15); 
    TEST_ASSERTM(q.getError()=="", q.getError());

    deleteAll(INFLUXDB_CLIENT_TESTING_URL);

    Serial.println("Stop server");
    waitServer(client, false);
    client.setHTTPOptions(HTTPOptions().httpReadTimeout(500));

    for (int i = 0; i < 25; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        //will succeed only first batchsize-1 points
        TEST_ASSERTM(client.writePoint(*p) == (i < 4), String("i=") + i);
        delete p;
    }
    TEST_ASSERT(client.isBufferFull());

    Serial.println("Start server");
    ;
    waitServer(client, true);
    client.setHTTPOptions(HTTPOptions().httpReadTimeout(5000));
    TEST_ASSERT(client.flushBuffer());
    q = client.query(query);
    std::vector<String> lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERT(lines.size() == 20);
    TEST_ASSERT(lines[0].indexOf(",5") > 0);
    TEST_ASSERT(lines[1].indexOf(",6") > 0);
    TEST_ASSERT(lines[2].indexOf(",7") > 0);
    TEST_ASSERT(lines[3].indexOf(",8") > 0);
    TEST_ASSERT(lines[18].indexOf(",23") > 0);
    TEST_ASSERT(lines[19].indexOf(",24") > 0);
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);

    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
}

void Test::testRetriesOnServerOverload() {
    TEST_INIT("testRetriesOnServerOverload");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    client.setWriteOptions(WriteOptions().batchSize(5).bufferSize(20).flushInterval(60));

    waitServer(client, true);
    TEST_ASSERT(client.validateConnection());
    for (int i = 0; i < 60; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        TEST_ASSERTM(client.writePoint(*p), String("i=") + i);
        delete p;
    }
    TEST_ASSERT(client.isBufferEmpty());
    String query = "select";
    FluxQueryResult q = client.query(query);
    TEST_ASSERT(countLines(q) == 60);
    TEST_ASSERTM(q.getError()=="", q.getError()); 
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);

    String rec = "a,direction=429-1 a=1";
    TEST_ASSERT(client.writeRecord(rec));
    TEST_ASSERT(!client.flushBuffer());
    client.resetBuffer();

    uint32_t start = millis();
    uint32_t retryDelay = 10;
    for (int i = 0; i < 52; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        uint32_t dur = (millis() - start) / 1000;
        if (client.writePoint(*p)) {
            if (i >= 4) {
                TEST_ASSERTM(dur >= retryDelay, String("Too early write: ") + dur);
            }
        } else {
            TEST_ASSERTM(i >= 4, String("i=") + i);
            if (dur >= retryDelay) {
                TEST_ASSERTM(false, String("Write should be ok: ") + dur);
            }
        }
        delete p;
        delay(333);
    }
    TEST_ASSERT(!client.isBufferEmpty());
    TEST_ASSERT(client.flushBuffer());
    TEST_ASSERT(client.isBufferEmpty());
    q = client.query(query);
    std::vector<String> lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERT(lines.size() == 37);  
    TEST_ASSERT(lines[0].indexOf(",15") > 0);
    TEST_ASSERT(lines[36].indexOf(",51") > 0);
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);

    // default retry
    rec = "a,direction=429-2 a=1";
    TEST_ASSERT(client.writeRecord(rec));
    TEST_ASSERT(!client.flushBuffer());
    client.resetBuffer();

    retryDelay = 5;
    start = millis();
    for (int i = 0; i < 52; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        uint32_t dur = (millis() - start) / 1000;
        if (client.writePoint(*p)) {
            if (i >= 4) {
                TEST_ASSERTM(dur >= retryDelay, String("Too early write: ") + dur);
            }
        } else {
            TEST_ASSERTM(i >= 4, String("i=") + i);
            if (dur >= retryDelay) {
                TEST_ASSERTM(false, String("Write should be ok: ") + dur);
            }
        }
        delete p;
        delay(164);
    }
    TEST_ASSERT(!client.isBufferEmpty());
    TEST_ASSERT(client.flushBuffer());
    TEST_ASSERT(client.isBufferEmpty());
    q = client.query(query);
    lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERT(lines.size() == 37);
    TEST_ASSERT(lines[0].indexOf(",15") > 0);
    TEST_ASSERT(lines[36].indexOf(",51") > 0);
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);

    rec = "a,direction=503-1 a=1";
    TEST_ASSERT(client.writeRecord(rec));
    TEST_ASSERT(!client.flushBuffer());
    client.resetBuffer();

    retryDelay = 10;
    start = millis();
    for (int i = 0; i < 52; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        uint32_t dur = (millis() - start) / 1000;
        if (client.writePoint(*p)) {
            if (i >= 4) {
                TEST_ASSERTM(dur >= retryDelay, String("Too early write: ") + dur);
            }
        } else {
            TEST_ASSERTM(i >= 4, String("i=") + i);
            if (dur >= retryDelay) {
                TEST_ASSERTM(false, String("Write should be ok: ") + dur);
            }
        }
        delete p;
        delay(1000);
    }
    TEST_ASSERT(!client.isBufferEmpty());
    TEST_ASSERT(client.flushBuffer());
    TEST_ASSERT(client.isBufferEmpty());
    q = client.query(query);
    lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERT(lines.size() == 52);
    TEST_ASSERT(lines[0].indexOf(",0") > 0);
    TEST_ASSERT(lines[51].indexOf(",51") > 0);
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);

    // default retry
    rec = "a,direction=503-2 a=1";
    TEST_ASSERT(client.writeRecord(rec));
    TEST_ASSERT(!client.flushBuffer());
    client.resetBuffer();

    retryDelay = 5;
    start = millis();
    for (int i = 0; i < 52; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        uint32_t dur = (millis() - start) / 1000;
        if (client.writePoint(*p)) {
            if (i >= 4) {
                TEST_ASSERTM(dur >= retryDelay, String("Too early write: ") + dur);
            }
        } else {
            TEST_ASSERTM(i >= 4, String("i=") + i);
            if (dur >= retryDelay) {
                TEST_ASSERTM(false, String("Write should be ok: ") + dur);
            }
        }
        delete p;
        delay(162);
    }
    TEST_ASSERT(!client.isBufferEmpty());
    TEST_ASSERT(client.flushBuffer());
    TEST_ASSERT(client.isBufferEmpty());

    q = client.query(query);
    lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERT(lines.size() == 37); 
    TEST_ASSERT(lines[0].indexOf(",15") > 0);
    TEST_ASSERT(lines[36].indexOf(",51") > 0);

    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
}

void Test::testFailedWrites() {
    TEST_INIT("testFailedWrites");

    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    client.setWriteOptions(WriteOptions().batchSize(1).bufferSize(5));
    //test with no batching
    TEST_ASSERT(client.validateConnection());
    for (int i = 0; i < 20; i++) {
        Point *p = createPoint("test1");
        if (!(i % 5)) {
            p->addTag("direction", "status");
            p->addTag("x-code", i > 10 ? "404" : "320");
        }
        p->addField("index", i);
        TEST_ASSERTM(client.writePoint(*p) == (i % 5 != 0), String("i=") + i + client.getLastErrorMessage());
        delete p;
    }
    int count;
    String query = "";
    FluxQueryResult q = client.query(query);
    std::vector<String> lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERT(lines.size() == 16);  //12 points+header
    TEST_ASSERTM(lines[0].indexOf(",1") > 0, lines[0]);
    TEST_ASSERTM(lines[4].indexOf(",6") > 0, lines[4]);
    TEST_ASSERTM(lines[9].indexOf(",12") > 0, lines[9]);
    TEST_ASSERTM(lines[15].indexOf(",19") > 0, lines[15]);
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);

    //test with batching
    client.setWriteOptions(WritePrecision::NoTime, 5, 20);
    for (int i = 0; i < 30; i++) {
        Point *p = createPoint("test1");
        if (!(i % 10)) {
            p->addTag("direction", "status");
            p->addTag("x-code", i > 10 ? "404" : "320");
        }
        p->addField("index", i);
        //i == 4,14,24 should fail
        TEST_ASSERTM(client.writePoint(*p) == ((i - 4) % 10 != 0), String("i=") + i);
        delete p;
    }

    q = client.query(query);
    lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    //3 batches should be skipped
    TEST_ASSERT(lines.size() == 15);  //15 points+header
    TEST_ASSERTM(lines[0].indexOf(",5") > 0, lines[0]);
    TEST_ASSERTM(lines[5].indexOf(",15") > 0, lines[5]);
    TEST_ASSERTM(lines[10].indexOf(",25") > 0, lines[10]);

    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
}

void Test::testTimestamp() {
    TEST_INIT("testTimestamp");

    struct timeval tv;
    tv.tv_usec = 1234;
    tv.tv_sec = 5678;
    unsigned long long ts = getTimeStamp(&tv, 0);
    TEST_ASSERTM( ts == 5678, timeStampToString(ts));
    ts = getTimeStamp(&tv, 3);
    TEST_ASSERTM( ts == 5678001, timeStampToString(ts));
    ts = getTimeStamp(&tv, 6);
    TEST_ASSERTM( ts == 5678001234, timeStampToString(ts));
    ts = getTimeStamp(&tv, 9);
    TEST_ASSERTM( ts == 5678001234000, timeStampToString(ts));

    // Test increasing timestamp
    String prev = "";
    for(int i = 0;i<2000;i++) {
        Point p("test");
        p.setTime(WritePrecision::US);
        String act = p.getTime();
        TEST_ASSERTM( i == 0 || prev < act, String(i) + ": " + prev + " vs " + act);
        prev = act;
        delayMicroseconds(100);
    }


    serverLog(INFLUXDB_CLIENT_TESTING_URL, "testTimestamp");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    client.setWriteOptions(WritePrecision::S, 1, 5);
    //test with no batching
    TEST_ASSERT(client.validateConnection());
    uint32_t timestamp;
    for (int i = 0; i < 20; i++) {
        Point *p = createPoint("test1");
        timestamp = time(nullptr);
        switch (i % 4) {
            case 0:
                p->setTime(timestamp);
                break;
            case 1: {
                String ts = String(timestamp);
                p->setTime(ts);
            } break;
            case 2:
                p->setTime(WritePrecision::S);
                break;
                //let other be set automatically
        }
        p->addField("index", i);
        TEST_ASSERTM(client.writePoint(*p), String("i=") + i);
        delete p;
    }
    int count;
    String query = "";
    FluxQueryResult q = client.query(query);
    std::vector<String> lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERT(lines.size() == 20);
    for (int i = 0; i < lines.size(); i++) {
        int partsCount;
        String *parts = getParts(lines[i], ',', partsCount);
        TEST_ASSERTM(partsCount == 11, String(i) + ":" + lines[i]);  //1measurement,4tags,5fields, 1timestamp
        parts[10].trim();
        TEST_ASSERTM(parts[10].length() == 10, String(i) + ":" + lines[i]);
        delete[] parts;
    }
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);

    client.setWriteOptions(WritePrecision::NoTime, 2, 5);
    //test with no batching
    for (int i = 0; i < 20; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        TEST_ASSERTM(client.writePoint(*p), String("i=") + i);
        delete p;
    }
    q = client.query(query);
    lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERT(lines.size() == 20);  //20 points+header
    for (int i = 0; i < lines.size(); i++) {
        int partsCount;
        String *parts = getParts(lines[i], ',', partsCount);
        TEST_ASSERTM(partsCount == 10, String(i) + ":" + lines[i]);  //1measurement,4tags,5fields
        delete[] parts;
    }

    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
    serverLog(INFLUXDB_CLIENT_TESTING_URL, "testTimestamp end");
}

void Test::testV1() {

    TEST_INIT("testV1");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_DB);

    TEST_ASSERTM(client.validateConnection(), client.getLastErrorMessage());
    //test with no batching
    for (int i = 0; i < 20; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        TEST_ASSERTM(client.writePoint(*p), String("i=") + i + client.getLastErrorMessage());
        delete p;
    }
    String query = "select";
    FluxQueryResult q = client.query(query);
    std::vector<String> lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERTM(lines.size() == 20, String(lines.size()) + " vs 20");
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
    
    //test with w/ batching 5
    client.setWriteOptions(WritePrecision::NoTime, 5);

    for (int i = 0; i < 15; i++) {
        Point *p = createPoint("test1");
        p->addField("index", i);
        TEST_ASSERTM(client.writePoint(*p), String("i=") + i + client.getLastErrorMessage());
        delete p;
    }
    q = client.query(query);
    lines = getLines(q);
    TEST_ASSERTM(q.getError()=="", q.getError());
    TEST_ASSERTM(lines.size() == 15, String(lines.size()));  

    // test precision
    for (int i = (int)WritePrecision::NoTime; i <= (int)WritePrecision::NS; i++) {
        client.setWriteOptions((WritePrecision)i, 1);
        Point *p = createPoint("test1");
        p->addField("index", i);
        TEST_ASSERTM(client.writePoint(*p), String("i=") + i);
        delete p;
    }
    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
}

void Test::testFluxTypes() {
    TEST_INIT("testFluxTypes");
    FluxValue val1;
    TEST_ASSERTM(val1.isNull(),"val1.isNull");
    TEST_ASSERTM(val1.getRawValue() == "","val1.getRawValue()");
    TEST_ASSERTM(val1.getString() == "","val1.getString()");
    FluxValue val2(new FluxLong("111", 111));
    TEST_ASSERTM(!val2.isNull(),"!val2.isNull");
    TEST_ASSERTM(val2.getLong() == 111,"val2.getLong");
    TEST_ASSERTM(val2.getRawValue() == "111","val2.getRawValue");
    TEST_ASSERTM(val2.getUnsignedLong() == 0,"val2.getUnsignedLong");
    TEST_ASSERTM(val2.getString() == "","val2.getString()");
    val1 = val2;
    TEST_ASSERTM(!val1.isNull(),"!val1.isNull");
    TEST_ASSERTM(val1.getLong() == 111,"val1.getLong");
    TEST_ASSERTM(val1.getRawValue() == "111","val1.getRawValue");
    TEST_ASSERTM(val1.getString() == "","val1.getString()");
    val2 = nullptr;
    TEST_ASSERTM(val2.isNull(),"val2.isNull");
    TEST_ASSERTM(!val1.isNull(),"!val1.isNull");
    TEST_ASSERTM(val1.getLong() == 111,"val1.getLong");
    TEST_ASSERTM(val1.getRawValue() == "111","val1.getRawValue");
    TEST_ASSERTM(val1.getString() == "","val1.getString()");

    FluxValue val3(new FluxUnsignedLong("123456", 123456));
    TEST_ASSERTM(!val3.isNull(),"!val3.isNull");
    TEST_ASSERTM(val3.getUnsignedLong() == 123456,"val3.getUnsignedLong");
    TEST_ASSERTM(val3.getRawValue() == "123456","val3.getRawValue");
    TEST_ASSERTM(val3.getLong() == 0,"val3.getLong");
    TEST_ASSERTM(val3.getString() == "","val3.getString()");

    val2 = val3;
    TEST_ASSERTM(!val2.isNull(),"!val2.isNull");
    TEST_ASSERTM(val2.getUnsignedLong() == 123456,"val2.getUnsignedLong");
    TEST_ASSERTM(val2.getRawValue() == "123456","val2.getRawValue");
    TEST_ASSERTM(val2.getLong() == 0,"val2.getLong");

    FluxValue val4(new FluxDouble("12.14", 12.14));
    TEST_ASSERTM(!val4.isNull(),"!val4.isNull");
    TEST_ASSERTM(val4.getDouble() == 12.14,"val4.getDouble");
    TEST_ASSERTM(val4.getLong() == 0,"val4.getLong");
    TEST_ASSERTM(val4.getRawValue() == "12.14","val4.getRawValue");
    TEST_ASSERTM(val4.getString() == "","val4.getString()");

    FluxValue val5(new FluxBool("true", true));
    TEST_ASSERTM(!val5.isNull(),"!val5.isNull");
    TEST_ASSERTM(val5.getBool(),"val5.getBool");
    TEST_ASSERTM(val5.getDouble() == 0.0,"val5.getDouble");
    TEST_ASSERTM(val5.getLong() == 0,"val45getLong");
    TEST_ASSERTM(val5.getRawValue() == "true","val5.getRawValue");
    TEST_ASSERTM(val5.getString() == "","val5.getString()");

    FluxValue val6(new FluxDateTime("2020-05-21T09:34:15.1234Z", FluxDatatypeDatetimeRFC3339, {15,34,9,21,4,120,0,0,0}, 123400));
    TEST_ASSERTM(!val6.isNull(),"!val6.isNull");
    TEST_ASSERTM(!val6.getBool(),"val6.getBool");
    TEST_ASSERTM(val6.getLong() == 0,"val6.getLong");
    TEST_ASSERTM(val6.getRawValue() == "2020-05-21T09:34:15.1234Z","val6.getRawValue");
    TEST_ASSERTM(val6.getString() == "","val6.getString()");
    struct tm t1 = {15,34,9,21,4,120,0,0,0};
    struct tm tx = val6.getDateTime().value;
    TEST_ASSERTM(compareTm(tx,t1), "val6.getDateTime().value");
    TEST_ASSERTM(val6.getDateTime().microseconds == 123400,"val6.getDateTime().microseconds");
    String dtStr = val6.getDateTime().format("%F %T");
    TEST_ASSERTM(dtStr == "2020-05-21 09:34:15",dtStr);


    FluxValue val7(new FluxDateTime("2020-05-22T09:34:15.123456Z", FluxDatatypeDatetimeRFC3339Nano, {15,34,9,22,4,120,0,0,0}, 123456));
    TEST_ASSERTM(!val7.isNull(),"!val7.isNull");
    TEST_ASSERTM(!val7.getBool(),"val7.getBool");
    TEST_ASSERTM(val7.getLong() == 0,"val7.getLong");
    TEST_ASSERTM(val7.getRawValue() == "2020-05-22T09:34:15.123456Z","val7.getRawValue");
    TEST_ASSERTM(val7.getString() == "","val7.getString()");
    struct tm t2 = {15,34,9,22,4,120,0,0,0};
    tx = val7.getDateTime().value;
    TEST_ASSERTM(compareTm(tx,t2), "val7.getDateTime().value");
    TEST_ASSERTM(val7.getDateTime().microseconds == 123456,"val7.getDateTime().microseconds");

    FluxValue val8(new FluxString("test string", FluxDatatypeString));
    TEST_ASSERTM(!val8.isNull(),"!val8.isNull");
    TEST_ASSERTM(!val8.getBool(),"val8.getBool");
    TEST_ASSERTM(val8.getLong() == 0,"val8.getLong");
    TEST_ASSERTM(val8.getRawValue() == "test string","val8.getRawValue");
    TEST_ASSERTM(val8.getString() == "test string","val8.getString()");

    FluxValue val9(new FluxString("1h4m5s", FluxDatatypeDuration));
    TEST_ASSERTM(!val9.isNull(),"!val9.isNull");
    TEST_ASSERTM(!val9.getBool(),"val9.getBool");
    TEST_ASSERTM(val9.getLong() == 0,"val9.getLong");
    TEST_ASSERTM(val9.getRawValue() == "1h4m5s","val9.getRawValue");
    TEST_ASSERTM(val9.getString() == "1h4m5s","val9.getString()");

    FluxValue val10(new FluxString("ZGF0YWluYmFzZTY0", FluxBinaryDataTypeBase64));
    TEST_ASSERTM(!val10.isNull(),"!val10.isNull");
    TEST_ASSERTM(!val10.getBool(),"val10.getBool");
    TEST_ASSERTM(val10.getLong() == 0,"val10.getLong");
    TEST_ASSERTM(val10.getRawValue() == "ZGF0YWluYmFzZTY0","val10.getRawValue");
    TEST_ASSERTM(val10.getString() == "ZGF0YWluYmFzZTY0","val10.getString()");
    TEST_END();
}

void Test::testFluxParserEmpty() {
    TEST_INIT("testFluxParserEmpty");
    FluxQueryResult flux("Error sss");
    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(flux.getError() == "Error sss","flux.getError");
    TEST_ASSERTM(flux.getValues().size() == 0,"flux.getValues().size()");
    TEST_ASSERTM(flux.getColumnsDatatype().size() == 0,"flux.getColumnsDatatype().size()");
    TEST_ASSERTM(flux.getColumnsName().size() == 0,"flux.getColumnsName().size()");
    TEST_ASSERTM(flux.getValueByIndex(0).isNull(),"flux.getValueByIndex(0).isNull()");
    TEST_ASSERTM(!flux.hasTableChanged(),"hasTableChanged");
    TEST_ASSERTM(flux.getTablePosition()==-1,"getTablePosition");
    TEST_ASSERTM(flux.getValueByName("xxx").isNull(),"flux.getValueByName(\"xxx\").isNull()");
    
    flux.close();
    // test unitialized
    InfluxDBClient client;
    flux = client.query("s");
    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(flux.getError() == "Unconfigured instance",flux.getError());

    flux.close();

    //test empty results set
    InfluxDBClient client2(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    TEST_ASSERT(waitServer(client2, true));
    flux = client2.query("testquery-empty");
    
    TEST_ASSERTM(!flux.next(),"flux.next()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());

    flux.close();

    TEST_END();
}

bool testFluxDateTimeValue(FluxQueryResult flux, int columnIndex,  const char *columnName, const char *rawValue, tm time, unsigned long us) {
    do {
        TEST_ASSERTM(flux.getValueByIndex(columnIndex).getRawValue() == rawValue, flux.getValueByName(columnName).getRawValue());
        FluxDateTime dt = flux.getValueByIndex(columnIndex).getDateTime();
        TEST_ASSERTM(compareTm(time, dt.value),  flux.getValueByIndex(columnIndex).getRawValue());
        TEST_ASSERTM(dt.microseconds == us,  String(dt.microseconds) + " vs " + String(us));
        dt = flux.getValueByName(columnName).getDateTime();
        TEST_ASSERTM(compareTm(time, dt.value),  flux.getValueByName(columnName).getRawValue());
        TEST_ASSERTM(dt.microseconds == us,  String(dt.microseconds) + " vs " + String(us));
        return true;
    } while(0);
end:
    return false;
}

bool testStringValue(FluxQueryResult flux, int columnIndex,  const char *columnName, const char *rawValue) {
    do {
        TEST_ASSERTM(flux.getValueByIndex(columnIndex).getString() == rawValue, flux.getValueByIndex(columnIndex).getString());
        TEST_ASSERTM(flux.getValueByName(columnName).getString() == rawValue, flux.getValueByName(columnName).getString());
        TEST_ASSERTM(flux.getValueByName(columnName).getRawValue() == rawValue, flux.getValueByName(columnName).getRawValue());
        return true;
    } while(0);
end:
    return false;
}

bool testStringVector(std::vector<String> vect, const char *values[], int size) {
    do {
        TEST_ASSERTM(vect.size() == size, String(vect.size()));
        for(int i=0;i<size;i++) {
            if(vect[i] != values[i]) {
                Serial.print("assert failure: ");
                Serial.println(vect[i]);
                goto end;
            }
        }
        return true;
    } while(0);
end:
    return false;
}

bool testDoubleValue(FluxQueryResult flux, int columnIndex,  const char *columnName, const char *rawValue, double value) {
    do {
        TEST_ASSERTM(flux.getValueByIndex(columnIndex).getDouble() == value, String(flux.getValueByIndex(columnIndex).getDouble()));
        TEST_ASSERTM(flux.getValueByName(columnName).getDouble() == value, String(flux.getValueByName(columnName).getDouble()));
        TEST_ASSERTM(flux.getValueByName(columnName).getRawValue() == rawValue, flux.getValueByName(columnName).getRawValue());
        return true;
    } while(0);
end:
    return false;
}

bool testLongValue(FluxQueryResult flux, int columnIndex,  const char *columnName, const char *rawValue, long value) {
    do {
        TEST_ASSERTM(flux.getValueByIndex(columnIndex).getLong() == value, String(flux.getValueByIndex(columnIndex).getLong()));
        TEST_ASSERTM(flux.getValueByName(columnName).getLong() == value, String(flux.getValueByName(columnName).getLong()));
        TEST_ASSERTM(flux.getValueByName(columnName).getRawValue() == rawValue, flux.getValueByName(columnName).getRawValue());
        return true;
    } while(0);
end:
    return false;
}

bool testUnsignedLongValue(FluxQueryResult flux, int columnIndex,  const char *columnName, const char *rawValue, unsigned long value) {
    do {
        TEST_ASSERTM(flux.getValueByIndex(columnIndex).getUnsignedLong() == value, String(flux.getValueByIndex(columnIndex).getUnsignedLong()));
        TEST_ASSERTM(flux.getValueByName(columnName).getUnsignedLong() == value, String(flux.getValueByName(columnName).getUnsignedLong()));
        TEST_ASSERTM(flux.getValueByName(columnName).getRawValue() == rawValue, flux.getValueByName(columnName).getRawValue());
        return true;
    } while(0);
end:
    return false;
}


bool testTableColumns(FluxQueryResult flux,  const char *columns[], int columnsCount) {
    do {
        TEST_ASSERT(testStringVector(flux.getColumnsName(), columns, columnsCount));
        for(int i=0;i<columnsCount;i++) {
            TEST_ASSERTM(flux.getColumnIndex(columns[i]) == i, columns[i]);
        }
        TEST_ASSERTM(flux.getColumnIndex("x") == -1, "flux.getColumnIndex(\"x\")");
        return true;
    } while(0);
end:
    return false;
}

void Test::testFluxParserSingleTable() {
    TEST_INIT("testFluxParserSingleTable");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    TEST_ASSERT(waitServer(client, true));
    FluxQueryResult flux = client.query("testquery-singleTable");
    TEST_ASSERTM(flux.next(),"flux.next()");
    TEST_ASSERTM(flux.hasTableChanged(),"flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());

    const char *types[] = {"string","long", "dateTime:RFC3339",  "dateTime:RFC3339",  "dateTime:RFC3339", "double", "string","string","string","string"};
    TEST_ASSERT(testStringVector(flux.getColumnsDatatype(), types, 10));
    const char *columns[] = {"result","table", "_start", "_stop", "_time", "_value", "_field","_measurement","a","b"};
    TEST_ASSERT(testTableColumns(flux, columns, 10));

    TEST_ASSERTM(flux.getValues().size() == 10,"flux.getValues().size() " + String(flux.getValues().size()));

    TEST_ASSERT(testStringValue(flux, 0, "result", ""));
    TEST_ASSERT(testLongValue(flux, 1, "table", "0", 0));
    TEST_ASSERT(testFluxDateTimeValue(flux, 2, "_start",  "2020-02-17T22:19:49.747562847Z", {49,19,22,17,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 3, "_stop",  "2020-02-18T22:19:49.747562847Z", {49,19,22,18,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 4, "_time",  "2020-02-18T10:34:08.135814545Z", {8,34,10,18,1,120,0,0,0}, 135814));
    TEST_ASSERT(testDoubleValue(flux, 5, "_value", "1.4", 1.4));
    TEST_ASSERT(testStringValue(flux, 6, "_field", "f"));
    TEST_ASSERT(testStringValue(flux, 7, "_measurement", "test"));
    TEST_ASSERT(testStringValue(flux, 8, "a", "1"));
    TEST_ASSERT(testStringValue(flux, 9, "b", "adsfasdf"));
    

    TEST_ASSERTM(flux.next(),"flux.next()");
    TEST_ASSERTM(!flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());

    TEST_ASSERT(testStringVector(flux.getColumnsDatatype(), types, 10));
    TEST_ASSERT(testTableColumns(flux, columns, 10));

    TEST_ASSERTM(flux.getValues().size() == 10,"flux.getValues().size() " + String(flux.getValues().size()));

    TEST_ASSERT(testStringValue(flux, 0, "result", ""));
    TEST_ASSERT(testLongValue(flux, 1, "table", "1", 1));
    TEST_ASSERT(testFluxDateTimeValue(flux, 2, "_start", "2020-02-17T22:19:49.747562847Z", {49,19,22,17,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 3, "_stop", "2020-02-18T22:19:49.747562847Z", {49,19,22,18,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 4, "_time", "2020-02-18T22:08:44.850214724Z", {44,8,22,18,1,120,0,0,0}, 850214));
    TEST_ASSERT(testDoubleValue(flux, 5, "_value", "6.6", 6.6));
    TEST_ASSERT(testStringValue(flux, 6, "_field", "f"));
    TEST_ASSERT(testStringValue(flux, 7, "_measurement", "test"));
    TEST_ASSERT(testStringValue(flux, 8, "a", "3"));
    TEST_ASSERT(testStringValue(flux, 9, "b", "adsfasdf"));

    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(!flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());

    flux.close();

    TEST_END();
}

void Test::testFluxParserNilValue() {
    TEST_INIT("testFluxParserNilValue");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    TEST_ASSERT(waitServer(client, true));
    FluxQueryResult flux = client.query("testquery-nil-value");
    TEST_ASSERTM(flux.next(),"flux.next()");
    TEST_ASSERTM(flux.hasTableChanged(),"flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());
    
    const char *types[] = {"string","long", "dateTime:RFC3339",  "dateTime:RFC3339",  "dateTime:RFC3339", "double", "string","string","string","string"};
    TEST_ASSERT(testStringVector(flux.getColumnsDatatype(), types, 10));
    const char *columns[] = {"result","table", "_start", "_stop", "_time", "_value", "_field","_measurement","a","b"};
    TEST_ASSERT(testTableColumns(flux, columns, 10));

    TEST_ASSERTM(flux.getColumnsName().size() == 10,"flux.getColumnsName().size()");
    
    TEST_ASSERTM(flux.getValueByIndex(5).isNull(), String(flux.getValueByIndex(5).isNull()));
    TEST_ASSERT(testDoubleValue(flux, 5, "_value", "", 0.0));

    TEST_ASSERTM(flux.next(),"flux.next()");
    TEST_ASSERTM(!flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());

    TEST_ASSERTM(!flux.getValueByIndex(5).isNull(), String(flux.getValueByIndex(5).isNull()));
    TEST_ASSERT(testDoubleValue(flux, 5, "_value", "6.6", 6.6));

    TEST_ASSERTM(flux.getValueByIndex(8).isNull(), String(flux.getValueByIndex(8).isNull()));
    TEST_ASSERT(testStringValue(flux, 8, "a", ""));

    TEST_ASSERTM(flux.next(),"flux.next()");
    TEST_ASSERTM(!flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());

    TEST_ASSERTM(!flux.getValueByIndex(5).isNull(), String(flux.getValueByIndex(5).isNull()));
    TEST_ASSERT(testDoubleValue(flux, 5, "_value", "1122.45", 1122.45));

    TEST_ASSERTM(!flux.getValueByIndex(8).isNull(), String(flux.getValueByIndex(8).isNull()));
    TEST_ASSERT(testStringValue(flux, 8, "a", "3"));

    TEST_ASSERTM(flux.getValueByIndex(9).isNull(), String(flux.getValueByIndex(9).isNull()));
    TEST_ASSERT(testStringValue(flux, 9, "b", ""));

    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(!flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());

    flux.close();

    TEST_END();
}

void Test::testFluxParserMultiTables(bool chunked) {
    TEST_INIT("testFluxParserMultiTables");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    TEST_ASSERT(waitServer(client, true));
    if(chunked) {
        String record = "a,direction=chunked a=1";
        client.writeRecord(record);
    }
    FluxQueryResult flux = client.query("testquery-multiTables");
    TEST_ASSERTM(flux.next(),"flux.next()");
    TEST_ASSERTM(flux.hasTableChanged(),"flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());
    // ===== table 1 =================
    
    const char *types[] = {"string","long", "dateTime:RFC3339",  "dateTime:RFC3339",  "dateTime:RFC3339", "unsignedLong", "string","string","string","string"};
    TEST_ASSERT(testStringVector(flux.getColumnsDatatype(), types, 10));
    const char *columns[] = {"result","table", "_start", "_stop", "_time", "_value", "_field","_measurement","a","b"};
    TEST_ASSERT(testTableColumns(flux, columns, 10));

    // ==== row 1 ========
    TEST_ASSERTM(flux.getValues().size() == 10,"flux.getValues().size() " + String(flux.getValues().size()));
    TEST_ASSERT(testStringValue(flux, 0, "result", "_result"));
    TEST_ASSERT(testLongValue(flux, 1, "table","0", 0));
    TEST_ASSERT(testFluxDateTimeValue(flux, 2, "_start", "2020-02-17T22:19:49.747562847Z", {49,19,22,17,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 3, "_stop", "2020-02-18T22:19:49.747562847Z", {49,19,22,18,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 4, "_time", "2020-02-18T10:34:08.135814545Z", {8,34,10,18,1,120,0,0,0}, 135814));
    TEST_ASSERT(testUnsignedLongValue(flux, 5, "_value", "14", 14));
    TEST_ASSERT(testStringValue(flux, 6, "_field", "f"));
    TEST_ASSERT(testStringValue(flux, 7, "_measurement", "test"));
    TEST_ASSERT(testStringValue(flux, 8, "a", "1"));
    TEST_ASSERT(testStringValue(flux, 9, "b", "adsfasdf"));
    //================= row 2 =========================
    TEST_ASSERTM(flux.next(),"flux.next()");
    TEST_ASSERTM(!flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());

    TEST_ASSERT(testStringVector(flux.getColumnsDatatype(), types, 10));
    TEST_ASSERT(testTableColumns(flux, columns, 10));

    TEST_ASSERTM(flux.getValues().size() == 10,"flux.getValues().size() " + String(flux.getValues().size()));
    TEST_ASSERT(testStringValue(flux, 0, "result", "_result"));
    TEST_ASSERT(testLongValue(flux, 1, "table","0", 0));
    TEST_ASSERT(testFluxDateTimeValue(flux, 2, "_start", "2020-02-17T22:19:49.747562847Z", {49,19,22,17,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 3, "_stop", "2020-02-18T22:19:49.747562847Z", {49,19,22,18,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 4, "_time", "2020-02-18T22:08:44.850214724Z", {44,8,22,18,1,120,0,0,0}, 850214));
    TEST_ASSERT(testUnsignedLongValue(flux, 5, "_value", "66", 66));
    TEST_ASSERT(testStringValue(flux, 6, "_field", "f"));
    TEST_ASSERT(testStringValue(flux, 7, "_measurement", "test"));
    TEST_ASSERT(testStringValue(flux, 8, "a", "1"));
    TEST_ASSERT(testStringValue(flux, 9, "b", "adsfasdf"));

    TEST_ASSERTM(flux.next(),"flux.next():" + flux.getError());
    TEST_ASSERTM(flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());
    
    // ===== table 2 =================
    const char *types2[] = {"string","long", "dateTime:RFC3339",  "dateTime:RFC3339",  "dateTime:RFC3339", "long", "string","string","string","string"};
    TEST_ASSERT(testStringVector(flux.getColumnsDatatype(), types2, 10));
    TEST_ASSERT(testTableColumns(flux, columns, 10));
    // ========== row 1 ================
    TEST_ASSERTM(flux.getValues().size() == 10,"flux.getValues().size() " + String(flux.getValues().size()));
    TEST_ASSERT(testStringValue(flux, 0, "result", "_result1"));
    TEST_ASSERT(testLongValue(flux, 1, "table","1", 1));
    TEST_ASSERT(testFluxDateTimeValue(flux, 2, "_start", "2020-02-16T22:19:49.747562847Z", {49,19,22,16,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 3, "_stop", "2020-02-17T22:19:49.747562847Z", {49,19,22,17,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 4, "_time", "2020-02-17T10:34:08.135814545Z", {8,34,10,17,1,120,0,0,0}, 135814));
    TEST_ASSERT(testLongValue(flux, 5, "_value", "-4", -4));
    TEST_ASSERT(testStringValue(flux, 6, "_field", "i"));
    TEST_ASSERT(testStringValue(flux, 7, "_measurement", "test"));
    TEST_ASSERT(testStringValue(flux, 8, "a", "1"));
    TEST_ASSERT(testStringValue(flux, 9, "b", "adsfasdf"));
    // === row 2 ==========
    TEST_ASSERTM(flux.next(),"flux.next()");
    TEST_ASSERTM(!flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());
    
    TEST_ASSERT(testStringVector(flux.getColumnsDatatype(), types2, 10));
    TEST_ASSERT(testTableColumns(flux, columns, 10));

    TEST_ASSERTM(flux.getValues().size() == 10,"flux.getValues().size() " + String(flux.getValues().size()));
    TEST_ASSERT(testStringValue(flux, 0, "result", "_result1"));
    TEST_ASSERT(testLongValue(flux, 1, "table", "1", 1));
    TEST_ASSERT(testFluxDateTimeValue(flux, 2, "_start", "2020-02-16T22:19:49.747562847Z", {49,19,22,16,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 3, "_stop", "2020-02-17T22:19:49.747562847Z", {49,19,22,17,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 4, "_time", "2020-02-16T22:08:44.850214724Z", {44,8,22,16,1,120,0,0,0}, 850214));
    TEST_ASSERT(testLongValue(flux, 5, "_value", "-1", -1));
    TEST_ASSERT(testStringValue(flux, 6, "_field", "i"));
    TEST_ASSERT(testStringValue(flux, 7, "_measurement", "test"));
    TEST_ASSERT(testStringValue(flux, 8, "a", "1"));
    TEST_ASSERT(testStringValue(flux, 9, "b", "adsfasdf"));


    TEST_ASSERTM(flux.next(),flux.getError());
    TEST_ASSERTM(flux.hasTableChanged(),flux.getError());
    TEST_ASSERTM(flux.getError() == "",flux.getError());

     // ===== table 3 =================
    const char *types3[] = {"string","long", "dateTime:RFC3339",  "dateTime:RFC3339",  "dateTime:RFC3339", "boolean", "string","string","string","string"};
    TEST_ASSERT(testStringVector(flux.getColumnsDatatype(), types3, 10));
    TEST_ASSERT(testTableColumns(flux, columns, 10));
    // ========== row 1 ================
    TEST_ASSERTM(flux.getValues().size() == 10,"flux.getValues().size() " + String(flux.getValues().size()));
    TEST_ASSERT(testStringValue(flux, 0, "result", "_result2"));
    TEST_ASSERT(testLongValue(flux, 1, "table", "2", 2));
    TEST_ASSERT(testFluxDateTimeValue(flux, 2, "_start", "2020-02-17T22:19:49.747562847Z", {49,19,22,17,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 3, "_stop", "2020-02-18T22:19:49.747562847Z", {49,19,22,18,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 4, "_time", "2020-02-18T10:34:08.135814545Z", {8,34,10,18,1,120,0,0,0}, 135814));
   
    TEST_ASSERTM(!flux.getValueByIndex(5).getBool(), String(flux.getValueByIndex(5).getBool()));
    TEST_ASSERTM(!flux.getValueByName("_value").getBool(), String(flux.getValueByName("_value").getBool()));
    TEST_ASSERTM(flux.getValueByName("_value").getRawValue() == "false", flux.getValueByName("_value").getRawValue());

    TEST_ASSERT(testStringValue(flux, 6, "_field", "b"));
    TEST_ASSERT(testStringValue(flux, 7, "_measurement", "test"));
    TEST_ASSERT(testStringValue(flux, 8, "a", "0"));
    TEST_ASSERT(testStringValue(flux, 9, "b", "brtfgh"));
    
    TEST_ASSERTM(flux.next(),"flux.next()");
    TEST_ASSERTM(!flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());
    //=== row 2 ====
    TEST_ASSERT(testStringVector(flux.getColumnsDatatype(), types3, 10));
    TEST_ASSERT(testTableColumns(flux, columns, 10));
    TEST_ASSERTM(flux.getValues().size() == 10,"flux.getValues().size() " + String(flux.getValues().size()));
    TEST_ASSERT(testStringValue(flux, 0, "result", "_result2"));
    TEST_ASSERT(testLongValue(flux, 1, "table", "2", 2));
    TEST_ASSERT(testFluxDateTimeValue(flux, 2, "_start", "2020-02-17T22:19:49.747562847Z", {49,19,22,17,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 3, "_stop", "2020-02-18T22:19:49.747562847Z", {49,19,22,18,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 4, "_time", "2020-02-18T22:08:44.969100374Z", {44,8,22,18,1,120,0,0,0}, 969100));
   
    TEST_ASSERTM(flux.getValueByIndex(5).getBool(), String(flux.getValueByIndex(5).getBool()));
    TEST_ASSERTM(flux.getValueByName("_value").getBool(), String(flux.getValueByName("_value").getBool()));
    TEST_ASSERTM(flux.getValueByName("_value").getRawValue() == "true", flux.getValueByName("_value").getRawValue());

    TEST_ASSERT(testStringValue(flux, 6, "_field", "b"));
    TEST_ASSERT(testStringValue(flux, 7, "_measurement", "test"));
    TEST_ASSERT(testStringValue(flux, 8, "a", "0"));
    TEST_ASSERT(testStringValue(flux, 9, "b", "brtfgh"));

    TEST_ASSERTM(flux.next(),"flux.next()");
    TEST_ASSERTM(flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());
     // ===== table 4 =================
    const char *types4[] = {"string","long", "dateTime:RFC3339Nano",  "dateTime:RFC3339Nano",  "dateTime:RFC3339Nano", "duration", "string","string","string","base64Binary"};
    TEST_ASSERT(testStringVector(flux.getColumnsDatatype(), types4, 10));
    TEST_ASSERT(testTableColumns(flux, columns, 10));
    // ========== row 1 ================
    TEST_ASSERTM(flux.getValues().size() == 10,"flux.getValues().size() " + String(flux.getValues().size()));
    TEST_ASSERT(testStringValue(flux, 0, "result", "_result3"));
    TEST_ASSERT(testLongValue(flux, 1, "table", "3", 3));
    TEST_ASSERT(testFluxDateTimeValue(flux, 2, "_start", "2020-02-10T22:19:49.747562847Z", {49,19,22,10,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 3, "_stop", "2020-02-12T22:19:49.747562847Z", {49,19,22,12,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 4, "_time", "2020-02-11T10:34:08.135814545Z", {8,34,10,11,1,120,0,0,0}, 135814));
    TEST_ASSERT(testStringValue(flux, 5, "_value", "1d2h3m4s"));
    TEST_ASSERT(testStringValue(flux, 6, "_field", "d"));
    TEST_ASSERT(testStringValue(flux, 7, "_measurement", "test"));
    TEST_ASSERT(testStringValue(flux, 8, "a", "0"));
    TEST_ASSERT(testStringValue(flux, 9, "b", "eHh4eHhjY2NjY2NkZGRkZA=="));
     // ====  row 2 ====
    TEST_ASSERTM(flux.next(),"flux.next()");
    TEST_ASSERTM(!flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());
    
    TEST_ASSERT(testStringVector(flux.getColumnsDatatype(), types4, 10));
    TEST_ASSERT(testTableColumns(flux, columns, 10));
    
    TEST_ASSERTM(flux.getValues().size() == 10,"flux.getValues().size() " + String(flux.getValues().size()));
    TEST_ASSERT(testStringValue(flux, 0, "result", "_result3"));
    TEST_ASSERT(testLongValue(flux, 1, "table", "3", 3));
    TEST_ASSERT(testFluxDateTimeValue(flux, 2, "_start", "2020-02-10T22:19:49.747562847Z", {49,19,22,10,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 3, "_stop", "2020-02-12T22:19:49.747562847Z", {49,19,22,12,1,120,0,0,0}, 747562));
    TEST_ASSERT(testFluxDateTimeValue(flux, 4, "_time", "2020-02-12T22:08:44.969100374Z", {44,8,22,12,1,120,0,0,0}, 969100));
    TEST_ASSERT(testStringValue(flux, 5, "_value", "22h52s"));
    TEST_ASSERT(testStringValue(flux, 6, "_field", "d"));
    TEST_ASSERT(testStringValue(flux, 7, "_measurement", "test"));
    TEST_ASSERT(testStringValue(flux, 8, "a", "0"));
    TEST_ASSERT(testStringValue(flux, 9, "b", "ZGF0YWluYmFzZTY0"));

    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(!flux.hasTableChanged(),"!flux.hasTableChanged()");
    TEST_ASSERTM(flux.getError() == "",flux.getError());
    
    flux.close();

    TEST_END();
}

void Test::testFluxParserErrorDiffentColumnsNum() {
    TEST_INIT("testFluxParserErrorDiffentColumnsNum");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    TEST_ASSERT(waitServer(client, true));
    FluxQueryResult flux = client.query("testquery-diffNum-data");

    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(flux.getError() == "Parsing error, row has different number of columns than table: 11 vs 10",flux.getError());
    
    flux.close();

    flux = client.query("testquery-diffNum-type-vs-header");

    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(flux.getError() == "Parsing error, header has different number of columns than table: 9 vs 10",flux.getError());

    flux.close(); 

    TEST_END();
}

void Test::testFluxParserFluxError() {
    TEST_INIT("testFluxParserFluxError");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    TEST_ASSERT(waitServer(client, true));
    FluxQueryResult flux = client.query("testquery-flux-error");

    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(flux.getError() == "{\"code\":\"invalid\",\"message\":\"compilation failed: loc 4:17-4:86: expected an operator between two expressions\"}",flux.getError());
    
    flux.close();

    TEST_END();
}

void Test::testFluxParserInvalidDatatype() {
    TEST_INIT("testFluxParserInvalidDatatype");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    TEST_ASSERT(waitServer(client, true));
    FluxQueryResult flux = client.query("testquery-invalid-datatype");

    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(flux.getError() == "Unsupported datatype: int",flux.getError());
    
    flux.close();

    TEST_END();
}

void Test::testFluxParserMissingDatatype() {
    TEST_INIT("testFluxParserMissingDatatype");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    TEST_ASSERT(waitServer(client, true));
    FluxQueryResult flux = client.query("testquery-missing-datatype");

    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(flux.getError() == "Parsing error, datatype annotation not found",flux.getError());
    
    flux.close();

    TEST_END();
}

void Test::testFluxParserErrorInRow() {
    TEST_INIT("testFluxParserErrorInRow");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    TEST_ASSERT(waitServer(client, true));
    FluxQueryResult flux = client.query("testquery-error-it-row-full");

    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(flux.getError() == "failed to create physical plan: invalid time bounds from procedure from: bounds contain zero time,897",flux.getError());
    
    flux.close();

    flux = client.query("testquery-error-it-row-no-reference");

    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(flux.getError() == "failed to create physical plan: invalid time bounds from procedure from: bounds contain zero time",flux.getError());
    
    flux.close();

    flux = client.query("testquery-error-it-row-no-message");

    TEST_ASSERTM(!flux.next(),"!flux.next()");
    TEST_ASSERTM(flux.getError() == "Unknown query error",flux.getError());
    
    flux.close();

    TEST_END();
}

void Test::testRetryInterval() {
    TEST_INIT("testRetryInterval");
    InfluxDBClient client(INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_TOK);
    client.setWriteOptions(WriteOptions().retryInterval(2));

    
    waitServer(client, true);
    TEST_ASSERT(client.validateConnection());

    String rec = "test1,direction=permanent-set,x-code=502,SSID=bonitoo.io,device_name=ESP32,device_id=4272205360 temperature=28.60,humidity=86i,code=69i,door=false,status=\"failed\",index=0";
    TEST_ASSERT(!client.writeRecord(rec));
    TEST_ASSERTM(client._lastRetryAfter == 2, String(client._lastRetryAfter));
    TEST_ASSERTM(client._writeBuffer[0]->retryCount == 1, String(client._writeBuffer[0]->retryCount));
    delay(2000);
    rec = "test1,direction=permanent-unset,SSID=bonitoo.io,device_name=ESP32,device_id=4272205360 temperature=28.60,humidity=86i,code=69i,door=false,status=\"failed\",index=2";
    TEST_ASSERT(!client.writeRecord(rec));
    TEST_ASSERTM(client._lastRetryAfter == 4, String(client._lastRetryAfter));
    TEST_ASSERTM(client._writeBuffer[0]->retryCount == 2, String(client._writeBuffer[0]->retryCount));
    delay(4000);
    rec = "test1,SSID=bonitoo.io,device_name=ESP32,device_id=4272205360 temperature=28.60,humidity=86i,code=69i,door=false,status=\"failed\",index=3";
    TEST_ASSERT(!client.writeRecord(rec));
    TEST_ASSERTM(client._lastRetryAfter == 8, String(client._lastRetryAfter));
    TEST_ASSERTM(client._writeBuffer[0]->retryCount == 3, String(client._writeBuffer[0]->retryCount));
    delay(8000);
    rec = "test1,SSID=bonitoo.io,device_name=ESP32,device_id=4272205360 temperature=28.60,humidity=86i,code=69i,door=false,status=\"failed\",index=4";
    TEST_ASSERT(!client.writeRecord(rec));
    TEST_ASSERTM(client._lastRetryAfter == 2, String(client._lastRetryAfter));
    TEST_ASSERT(!client._writeBuffer[0]);
    TEST_ASSERTM(client._writeBuffer[1]->retryCount == 0, String(client._writeBuffer[1]->retryCount));

    delay(2000);
    rec = "test1,SSID=bonitoo.io,device_name=ESP32,device_id=4272205360 temperature=28.60,humidity=86i,code=69i,door=false,status=\"failed\",index=5";
    TEST_ASSERT(!client.writeRecord(rec));
    TEST_ASSERTM(client._lastRetryAfter == 2, String(client._lastRetryAfter));
    TEST_ASSERT(!client._writeBuffer[0]);
    TEST_ASSERTM(client._writeBuffer[1]->retryCount == 1, String(client._writeBuffer[1]->retryCount));

    delay(2000);
    TEST_ASSERTM(client.flushBuffer(), client.getLastErrorMessage());
    String query = "select";
    FluxQueryResult q = client.query(query);
    TEST_ASSERT(countLines(q) == 3); //point with the direction tag is skipped
    TEST_ASSERTM(q.getError()=="", q.getError()); 

    TEST_END();
    deleteAll(INFLUXDB_CLIENT_TESTING_URL);
}

Point *createPoint(String measurement) {
    Point *point = new Point(measurement);
    point->addTag("SSID", WiFi.SSID());
    point->addTag("device_name", deviceName);
    point->addTag("device_id", chipId);
    point->addField("temperature", random(-20, 40) * 1.1f);
    point->addField("humidity", random(10, 90));
    point->addField("code", random(10, 90));
    point->addField("door", random(0, 10) > 5);
    point->addField("status", random(0, 10) > 5 ? "ok" : "failed");
    return point;
}

void initInet() {
    int i = 0,j = 0;
    bool wifiOk = false;
    while(!wifiOk && j<3) {
        Serial.print("Connecting to wifi ");
        WiFi.begin(INFLUXDB_CLIENT_TESTING_SSID, INFLUXDB_CLIENT_TESTING_PASS);
        while ((WiFi.status() != WL_CONNECTED) && (i < 30)) {
            Serial.print(".");
            delay(300);
            i++;
        }
        Serial.println();
        wifiOk = WiFi.status() == WL_CONNECTED;
        if(!wifiOk) {
            WiFi.disconnect();
        }
        j++;
    }
    if (!wifiOk) {
        Serial.println("Wifi connection failed");
        Serial.println("Restating");
        ESP.restart();
    } else {
        Serial.printf("Connected to: %s (%d)\n", WiFi.SSID().c_str(), WiFi.RSSI());
        Serial.print("Ip: ");
        Serial.println(WiFi.localIP());

        timeSync("CET-1CEST,M3.5.0,M10.5.0/3", "0.cz.pool.ntp.org", "1.cz.pool.ntp.org", "pool.ntp.org");

        deleteAll(INFLUXDB_CLIENT_TESTING_URL);
    }
}