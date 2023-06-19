#include "ISAPI.h"

FILE* fp;

enum stream
{
    mainstream = 1,
    substream,
    thrstream
};

enum capabilities{
    
};
/// <summary>
/// 文件流操作方式，判断文件是否存在
/// </summary>
/// <param name="name"></param>
/// <returns></returns>
inline bool filestream_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

/// <summary>
/// 文件操作方式，判断文件是否存在
/// </summary>
/// <param name="name"></param>
/// <returns></returns>
inline bool file_exists(const std::string& name) {
    if (FILE* file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    }
    else {
        return false;
    }
}

/// <summary>
/// 把字符串转换成16进制字符串
/// </summary>
/// <param name="data"></param>
/// <param name="len"></param>
/// <returns></returns>
std::string to_hex(unsigned char* data, int len) {
    std::stringstream ss;
    ss << std::uppercase << std::hex << std::setfill('0');
    for (int i = 0; i < len; i++) {
        ss << std::setw(2) << static_cast<unsigned>(data[i]);
    }
    return ss.str();
}

/// <summary>
/// 从文件读入到string里
/// </summary>
/// <param name="filename"></param>
/// <returns></returns>
std::string readFileIntoString(char* filename)
{
    std::ifstream ifile(filename);
    //将文件读入到ostringstream对象buf中
    std::ostringstream buf;
    char ch;
    while (buf && ifile.get(ch))
        buf.put(ch);
    //返回与流对象buf关联的字符串
    return buf.str();
}

/// <summary>
/// 字符串剪切
/// </summary>
/// <param name="input"></param>
/// <param name="parmas"></param>
/// <returns></returns>
std::string strcut(std::string input, std::string parmas) {
    int pos = input.find(parmas);
    input = input.substr(pos + parmas.length());
    //std::cout << input << std::endl;
    return input;
}


static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
    std::string* str = dynamic_cast<std::string*>((std::string*)lpVoid);
    if (NULL == str || NULL == buffer)
    {
        return -1;
    }
    char* pData = (char*)buffer;
    str->append(pData, size * nmemb);
    return nmemb;
}

static size_t onHeader(char *buffer, size_t size, size_t nitems, void *userdata)
{
    /* received header is nitems * size long in 'buffer' NOT ZERO TERMINATED */
    /* 'userdata' is set with CURLOPT_HEADERDATA */
    string *str = (string *)userdata;
    if (str == NULL || buffer == NULL) {
        return -1;
    }
    str->append(buffer, size*nitems);
    return nitems * size;
}

ISAPI::ISAPI(){
    stream nstream = mainstream;
    readconfigFile();
    capabilities_Url = {
        {"系统能力获取",                        "http://" + ip + "/ISAPI/System/capabilities"},
        {"获取主动跟踪能力",                    "http://" + ip + "/ISAPI/MasterSlaveTracking/capabilities"},    //不支持
        {"获取所有串口能力",                    "http://" + ip + "/ISAPI/System/Serial/capabilities"},
        {"获取设备SDP信息",                     "rtsp://" + ip + "/ISAPI/Streaming/channels/101 RTSP/1.0"},
        {"获取通道号",                          "http://" + ip +"/ISAPI/Streaming/channels"},
        {"获取单个通道事件能力",                 "http://" + ip +"/ISAPI/Event/channels/" + PTZChannel + "/capabilities"},
        {"获取通道属性",                        "http://" + ip +"/ISAPI/AUXInfo/attributes/Channels"},
        {"获取设备类型",                        "http://" + ip +"/ISAPI/System/deviceInfo"},
        {"查询月历录像",                        "http://" + ip +"/ISAPI/ContentMgmt/record/tracks/" + std::to_string(atoi(PTZChannel.c_str())*100 + nstream) + "/dailyDistribution"},
        {"获取定时抓图任务能力集",               "http://" + ip +"/ISAPI/System/Video/scheduledCaptureTask/capabilities?format=json"},
        {"获取实时流定时抓图任务能力集",          "http://" + ip +"/ISAPI/System/Video/scheduledCaptureTask/realtimeStream/capabilities?format=json"},
        {"获取设备按目标类型进行检索的能力",      "http://" + ip +"/ISAPI/ContentMgmt/SearchByTargetType/capabilities?format=json"},
        {"获取设备支持录像查询能力",             "http://" + ip +"/ISAPI/ContentMgmt/capabilities"},
        {"全屏测温能力获取",                    "http://" + ip + "/ISAPI/Thermal/channels/" + ThermalChannel + "/thermometry/pixelToPixelParam/capabilities"},
        {"热成像码流能力获取",                  "http://" + ip + "/ISAPI/Thermal/channels/" + ThermalChannel + "/streamParam/capabilities"},
        {"云台能力获取",                        "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/capabilities"},
        {"获取云台所有辅助设备",                 "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/auxcontrols"},
        {"高精度绝对PTZ运动控制能力获取",        "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/RailwayRobot/capabilities?format=json"},
        {"绝对位置获取",                        "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/absoluteEx"},
        {"抓拍热图参数获取",                    "http://" + ip + "/ISAPI/Snapshot/channels/" + ThermalChannel},
        {"抓拍图参数获取",                      "http://" + ip + "/ISAPI/Snapshot/channels/" + PTZChannel },
        {"获取人脸比对能力",                    "http://" + ip + "/ISAPI/Intelligent/FDLib/capabilities"},
        {"获取设备支持录像查询能力节点",         "http://" + ip + "/ISAPI/ContentMgmt/capabilities"},
        {"获取全部预置点",                      "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/presets/"},
        {"获取监听主机参数能力",                "http://" + ip + "/ISAPI/Event/notification/httpHosts/capabilities"}
    };
    Action_Url = {
        {"定时抓拍",            "http://" + ip + "/ISAPI/System/Video/inputs/channels/" + PTZChannel + "/imageCapture/capabilities?format=json"},
        {"定时抓图",            "http://" + ip + "/ISAPI/Event/schedules/imageCaptures/imageCapture-1"},
        {"抓图",                 "http://" + ip + "/ISAPI/Image/channels/" + PTZChannel + "/imageCap"},
        {"可见光抓图",            "http://" + ip + "/ISAPI/Streaming/channels/" + PTZChannel + "/picture"},
        {"建立非订阅布防连接",   "http://" + ip + "/ISAPI/Event/notification/alertStream"},
        {"全屏测温能力配置",     "http://" + ip + "/ISAPI/Thermal/channels/" + ThermalChannel + "/thermometry/pixelToPixelParam"},
        {"抓热图",              "http://" + ip + "/ISAPI/Thermal/channels/" + ThermalChannel + "/thermometry/jpegPicWithAppendData?format=json"},
        {"热成像码流能力配置",   "http://" + ip + "/ISAPI/Thermal/channels/" + ThermalChannel + "/streamParam"},
        {"实时测温",            "http://" + ip + "/ISAPI/Thermal/channels/" + ThermalChannel + "/thermometry/realTimethermometry/rules?format=json"},
        {"云台绝对位置运动",     "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/absolute"},
        {"3D定位",              "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/position3D"},
        {"云台相对位置运动",     "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/continuous"},
        {"云台变倍速度",         "http://" + ip + "/ISAPI/Image/channels/" + PTZChannel + "/proportionalpan"},
        {"一键聚焦参数配置",     "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/onepushfoucs/start"},
        {"设置预置点",           "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/presets/"},
        {"巡航路劲管理",         "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/patrols"},
        {"重置云台一键聚焦参数",  "http://" + ip + "/ISAPI/PTZCtrl/channels/" + PTZChannel + "/onepushfoucs/reset"}
    };
    PTZ_Getpresets(capabilities_Url["获取全部预置点"]);
}



std::string ISAPI::RTSP(std::string url){
    CURLcode res;
    //In windows, this will init the winsock stuff
    res = curl_global_init(CURL_GLOBAL_ALL);
    if (res == CURLE_OK) {
        curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);
        fprintf(stderr, "    cURL V%s loaded\n", data->version);
        /* get a curl handle */
        CURL* mCurl = curl_easy_init();
        if (mCurl != NULL) {
            curl_easy_setopt(mCurl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(mCurl, CURLOPT_PROTOCOLS, (long)CURLPROTO_RTSP);
        }
        else {
            fprintf(stderr, "curl_easy_init() failed\n");
        }

        std::string responseStr;
        std::string ResponseData;
        std::string responseHeader;
        
        curl_easy_setopt(mCurl, CURLOPT_RTSP_STREAM_URI, url);
        curl_easy_setopt(mCurl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_DESCRIBE);
        //应答消息主体
        curl_easy_setopt(mCurl, CURLOPT_WRITEFUNCTION, OnWriteData);
        //打印到字符串
        curl_easy_setopt(mCurl, CURLOPT_WRITEDATA, (void *)&responseStr);
        //应答消息头部
        curl_easy_setopt(mCurl, CURLOPT_HEADERFUNCTION, onHeader);
        //打印到字符串
        curl_easy_setopt(mCurl, CURLOPT_HEADERDATA, (void *)&responseHeader);

        CURLcode nRet = curl_easy_perform(mCurl);
        if (0 == nRet)
        {

        }
        else
        {
            std::cout << "接收失败" << std::endl;
        }
        curl_easy_cleanup(mCurl);
        ResponseData = responseHeader;
        return ResponseData;
    }
    else {
        fprintf(stderr, "curl_global_init(%s) failed: %d\n",
            "CURL_GLOBAL_ALL", res);
    }
}

/// <summary>
/// 获取请求文件
/// </summary>
/// <param name="url"></param>
/// <param name="modth"></param>
/// <param name="strResponseData"></param>
/// <returns></returns>
std::string ISAPI::GET_XML(std::string url, std::string modth, std::string strResponseData)
{
    std::string ResponseData;
    CURL* pCurlHandle = curl_easy_init();
    if (modth == "PUT") {
        curl_easy_setopt(pCurlHandle, CURLOPT_POSTFIELDS, strResponseData.c_str());
        curl_easy_setopt(pCurlHandle, CURLOPT_CUSTOMREQUEST, modth.c_str());
    }
    else if (modth == "POST") {
        curl_easy_setopt(pCurlHandle, CURLOPT_POSTFIELDS, strResponseData.c_str());
        curl_easy_setopt(pCurlHandle, CURLOPT_CUSTOMREQUEST, modth.c_str());
    }
    else if (modth == "GET") {
        curl_easy_setopt(pCurlHandle, CURLOPT_CUSTOMREQUEST, modth.c_str());
    }
    else if(modth == "DELETE"){
        curl_easy_setopt(pCurlHandle, CURLOPT_CUSTOMREQUEST, modth.c_str());
    }
    curl_easy_setopt(pCurlHandle, CURLOPT_URL, url.c_str());
    // 设置用户名和密码
    curl_easy_setopt(pCurlHandle, CURLOPT_USERPWD, (username + ":" + password).c_str());
    // 设置认证方式为摘要认证
    curl_easy_setopt(pCurlHandle, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
    // 设置回调函数
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEFUNCTION, OnWriteData);
    // 设置回调函数的参数,获取反馈信息
    curl_easy_setopt(pCurlHandle, CURLOPT_WRITEDATA, &strResponseData);
    // 接收数据时超时设置，如果5秒内数据未接收完，直接退出
    curl_easy_setopt(pCurlHandle, CURLOPT_TIMEOUT, 5);
    // 设置重定向次数，防止重定向次数太多
    curl_easy_setopt(pCurlHandle, CURLOPT_MAXREDIRS, 1);
    // 连接超时，这个数值如果设置太短可能导致数据请求不到就断开了
    curl_easy_setopt(pCurlHandle, CURLOPT_CONNECTTIMEOUT, 5);
    CURLcode nRet = curl_easy_perform(pCurlHandle);
    if (0 == nRet)
    {

    }
    else
    {
        std::cout << "接收失败" << std::endl;
    }
    curl_easy_cleanup(pCurlHandle);
    ResponseData = strResponseData;
    return ResponseData;
}

/// <summary>
/// 判断是否支持指定功能
/// </summary>
/// <param name="url"></param>
/// <returns></returns>
bool ISAPI::PTZ_Getcapabilities(std::string url, std::string isSupport) {
    std::string request;
    std::string strResponseData;
    request = GET_XML(url, "GET", strResponseData);
    std::cout << request << std::endl;
    if (strstr(request.c_str(), isSupport.c_str()) != NULL) {
        std::stringstream Start_isSupport, End_isSupport;
        Start_isSupport << "<" << isSupport << ">";
        End_isSupport << "</" << isSupport << ">";
        std::string isSupport = request.substr(request.find(Start_isSupport.str()) + Start_isSupport.str().length(),
            request.find(End_isSupport.str()) - (request.find(Start_isSupport.str()) + Start_isSupport.str().length()));
        if (isSupport == "false") {
            return false;
        }
        else {
            return true;
        }
    }
    else {
        return false;
    }
}

void ISAPI::GetChannals(std::string ip){
    std::string request;
    std::string strResponseData;
    request = GET_XML(capabilities_Url["获取通道号"], "GET", strResponseData);
    std::cout << request << std::endl;
}

void ISAPI::ImageCapture(std::string Channel)
{
    std::string request;
    std::string strResponseData;
    std::string url = "http://" + ip + "/ISAPI/Streaming/channels/" + Channel + "/picture";
    request = GET_XML(url, "GET", strResponseData);
    //数据分割，获取图片
    char* begin = NULL;
    fp = fopen("./VisablePic.JPEG", "wb");
    fwrite(request.c_str(), request.length(), 1, fp);
    fclose(fp);
}

 void ISAPI::PTZ_continuous(std::string Channel, int P, int T, int Z, int R){
    if (PTZ_Getcapabilities(capabilities_Url["云台能力获取"], "ContinuousPanTiltSpace")){
        std::string request;
        std::string strResponseData = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                  "<PTZData xmlns=\"http://www.isapi.org/ver20/XMLSchema\" version=\"2.0\">\n"
                                    "<continuousHigh>\n" 
                                        /*"<!--opt, int, 海拔, range:[-900,2700]-->\n"*/
                                        "<pan>"
                                            + std::to_string(P) +
                                        "</pan>\n"
                                        /*"< !--opt, int, 方位角, range: [0, 3600] -->\n"*/
                                        "<tilt>"
                                            + std::to_string(T) +
                                        "</tilt>\n"
                                        /*"<!--opt, int, 绝对缩放, range:[0,1000]-->\n"*/
                                        "<zoom>"
                                            + std::to_string(Z) +
                                        "</zoom>\n"
                                        /*"<!--opt, int, 绝对旋转角度, range:[0,350]-->"
                                        "<absoluteRotate>"
                                            "1"
                                        "</absoluteRotate>"*/
                                    "</continuousHigh>\n"
                                  "</PTZData>\n";
        std::string url = "http://" + ip + "/ISAPI/PTZCtrl/channels/" + Channel + "/continuous";
        request = GET_XML(url, "PUT", strResponseData);
        std::cout << request << std::endl;
    }else{
        std::cout << "设备不支持" << std::endl;
    }
 }

void ISAPI::PTZ_Getpresets(std::string url){
    Preset p;
    std::string request;
    std::string strResponseData;
    std::vector<std::string> xml_preset;
    request = GET_XML(url, "GET", strResponseData);
    std::string Start_PTZPreset = "<PTZPreset>";
    std::string End_PTZPreset = "</PTZPreset>";
    while(strstr(request.c_str(), Start_PTZPreset.c_str()) != NULL){
        // std::cout << request << std::endl;
        xml_preset.push_back(request.substr(request.find(Start_PTZPreset),request.find(End_PTZPreset)+End_PTZPreset.length()-request.find(Start_PTZPreset)));
        
        request = request.substr(request.find(End_PTZPreset)+End_PTZPreset.length());
    }
    for(vector<std::string>::iterator iter = xml_preset.begin(); iter != xml_preset.end(); iter++){
        std::string strtemp = *iter;
        p.id = strtemp.substr(strtemp.find("<id>")+std::string("<id>").length(),strtemp.find("</id>")-strtemp.find("<id>")-std::string("<id>").length());
        p.presetName = strtemp.substr(strtemp.find("<presetName>")+std::string("<presetName>").length(),strtemp.find("</presetName>")-strtemp.find("<presetName>")-std::string("<presetName>").length());
        presets.push_back(p);
    }
    PTZ_PresetsMenu();
}

void ISAPI::PTZ_PresetsMenu(){
    int i = 0;
    for(vector<Preset>::iterator iter = presets.begin(); iter != presets.end(); iter++){
        std::cout << "数组序号:" << i << " PresetID:" << (*iter).id << " PresetName:" << (*iter).presetName << std::endl;
        std::cout << std::endl;
        i++;
    }
}

void ISAPI::PTZ_POSTPersets(std::string url, std::string enabled, std::string id, std::string presetName){
    std::string request;
    std::string strResponseData = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                    "<PTZPreset xmlns=\"http://www.isapi.org/ver20/XMLSchema\" version=\"2.0\">\n"
                                        "<enabled>"
                                        + enabled +
                                        "</enabled>\n"
                                        "<id>"
                                        + id +
                                        "</id>\n"
                                        "<presetName>"
                                        + presetName +
                                        "</presetName>\n"
                                    "</PTZPreset>";
    request = GET_XML(url, "POST", strResponseData);
    std::cout << request << std::endl;
}

void ISAPI::PTZ_PresetsGoto(std::string Channel, std::string presetID){
    std::string request;
    std::string strResponseData ;
    std::string url = "http://" + ip + "/ISAPI/PTZCtrl/channels/" + Channel + "/presets/" + presetID + "/goto";
    request = GET_XML(url, "PUT", strResponseData);
    std::cout << request << std::endl;
}

/// <summary>
/// 抓热图
/// </summary>
/// <param name="url"></param>
void ISAPI::jpegPicWithAppendData(std::string ThermalChannel, int x1, int y1, int x2, int y2) {
    if (PTZ_Getcapabilities(capabilities_Url["系统能力获取"], "isSupportJpegPicWithAppendData"))
    {
        std::string request;
        std::string strResponseData = "<?xml version=\"1.0\" encoding=\"UTF - 8\"?>"
            "<PixelToPixelParam version = \"2.0\" xmlns = \"http://www.hikvision.com/ver20/XMLSchema\">"
            "<id>2</id>"
            "<maxFrameRate>400</maxFrameRate>"
            "<reflectiveEnable>false</reflectiveEnable>"
            "<reflectiveTemperature>20.00</reflectiveTemperature>"
            "<emissivity>0.98</emissivity>"
            "<distance>3000</distance>"
            "<refreshInterval>50</refreshInterval>"
            "<distanceUnit>centimeter</distanceUnit>"
            "<temperatureDataLength>4</temperatureDataLength>"
            "<JpegPictureWithAppendData>"
            "<jpegPicEnabled>true</jpegPicEnabled>"
            "<visiblePicEnabled>true</visiblePicEnabled>"
            "</JpegPictureWithAppendData>"
            "</PixelToPixelParam>";
        std::string m_url = "http://" + ip + "/ISAPI/Thermal/channels/" + ThermalChannel + "/thermometry/pixelToPixelParam";
        request = GET_XML(m_url, "PUT", strResponseData);
        std::cout << request << std::endl;
        std::vector<std::string> strarr;
        std::string url = "http://" + ip + "/ISAPI/Thermal/channels/" + ThermalChannel + "/thermometry/jpegPicWithAppendData?format=json";
        request = GET_XML(url, "GET", strResponseData);
        //fp = fopen(file.c_str(), "wb");
        //fwrite(request.c_str(), request.length(), 1, fp);
        //fclose(fp);
        //request = readFileIntoString(const_cast<char *>(file.c_str()));
        //数据分割成多个部分
        while (strstr(request.c_str(), "--boundary") != NULL) {

            request = strcut(request, "--boundary");
            strarr.push_back(request.substr(0, request.find("--boundary")));
        }

        //数据分割，获取图片
        char* begin = NULL;
        int jpeglen;
        begin = const_cast<char*>(strstr(strarr[1].c_str(), "Content-Length:"));
        //printf("\n%s\n", begin);
        jpeglen = atoi(begin + 15);
        begin = const_cast<char*>(strstr(strarr[1].c_str(), "\r\n\r\n"));
        fp = fopen("./ThermalPic.JPEG", "wb");
        fwrite(begin + 4, jpeglen, 1, fp);
        fclose(fp);

        //获取图片像素，为后面坐标换算作准备
        std::string PIC_request;
        PIC_request = GET_XML(capabilities_Url["抓拍热图参数获取"], "GET", strResponseData);
        int pictureWidth = atoi(PIC_request.substr(PIC_request.find("<pictureWidth>") + std::string("<pictureWidth>").length(),
            PIC_request.find("</pictureWidth>") - (PIC_request.find("<pictureWidth>") + std::string("<pictureWidth>").length())).c_str());
        int pictureHeight = atoi(PIC_request.substr(PIC_request.find("<pictureHeight>") + std::string("<pictureHeight>").length(),
            PIC_request.find("</pictureHeight>") - (PIC_request.find("<pictureHeight>") + std::string("<pictureHeight>").length())).c_str());

        //获取温度数据
        int lines, row, column;
        int formlength = atoi(strstr(strarr[0].c_str(), "\"temperatureDataLength\":") + 24);
        lines = atoi(strstr(strarr[0].c_str(), "\"p2pDataLen\":") + 13) / formlength;  // 数组数据总数
        row = atoi(strstr(strarr[0].c_str(), "\"jpegPicWidth\":") + 15);       // 行
        column = atoi(strstr(strarr[0].c_str(), "\"jpegPicHeight\":") + 16);                 // 列
        int TemperatureDataLength;
        std::string TemperatureData;
        //温度数据与图片数据混合在一起是乱码，通过图片大小剪切出省下的数据部分
        TemperatureData = request.substr(request.find("Content-Length:") + 19 + jpeglen);
        //通过--boundary字符分割出温度数据部分
        TemperatureData = strcut(TemperatureData, "--boundary");
        begin = const_cast<char*>(strstr(TemperatureData.c_str(), "Content-Length:"));
        TemperatureDataLength = atoi(begin + 15);
        TemperatureData = TemperatureData.substr(TemperatureData.find("\r\n\r\n") + 4, TemperatureDataLength + TemperatureData.find("\r\n\r\n") + 4);

        if (TemperatureDataLength > 0 && TemperatureData != " ")
        {
            FILE* fAppendAata = fopen("Jpegwithappend.data", "wb");
            fwrite(TemperatureData.c_str(), TemperatureDataLength, 1, fAppendAata);
            fclose(fAppendAata);
        }

        FILE* datafilefp = fopen("Jpegwithappend.data", "rb+");
        if (NULL == datafilefp)
        {
            printf("Failed to open the file\n");
            return;
        }

        int m_AnalysisHotPic_W = row; //根据抓热图接口返回的图像宽dwJpegPicWidth进行赋值
        int m_AnalysisHotPic_H = column; //根据抓热图接口返回的图像高dwJpegPicHeight进行赋值
        int bufSize = m_AnalysisHotPic_H * m_AnalysisHotPic_W * 4;

        char* pDataBuf = (char*)malloc(bufSize);
        if (NULL == pDataBuf)
        {
            return;
        }
        //读取文件里面所有数据
        fread(pDataBuf, bufSize, 1, datafilefp);

        //保存分析结果的文件
        FILE* fp = fopen("JpegwithappendResult.csv", "wb");
        if (NULL == fp)
        {
            return;
        }

        int iIndex = 0;
        int flage_max_Width = 0, flage_min_Width = 0;
        int flage_max_Height = 0, flage_min_Height = 0;
        char temp[10] = { 0 };
        float minTemp = 10000.0;
        float maxTemp = -10000.0;
        int start_W = ((float)x1 / (float)(pictureWidth)) * m_AnalysisHotPic_W;
        int start_H = ((float)y1 / (float)(pictureHeight)) * m_AnalysisHotPic_H;
        //(start_H-1) * m_AnalysisHotPic_W + start_W
        int end_W = ((float)x2 / (float)(pictureWidth)) * m_AnalysisHotPic_W;
        int end_H = ((float)y2 / (float)(pictureHeight)) * m_AnalysisHotPic_H;

        int start_point = 0;
        for (int iWriteHeight = 0; iWriteHeight < m_AnalysisHotPic_H; ++iWriteHeight)
        {
            int position = 0;
            for (int iWriteWidth = 0; iWriteWidth < m_AnalysisHotPic_W; ++iWriteWidth)
            {
                float fTemp = *((float*)(pDataBuf + iIndex));
                /*std::cout << pDataBuf << std::endl;*/

                if ((position >= start_W && position <= end_W && iWriteHeight >= start_H && iWriteHeight <= end_H) || (start_W == end_W && start_H == end_H)) {
                    //判断fTemp是否是一个正常值，不是则赋值最大或最小
                    fTemp = (9999 < fTemp) ? 9999 : ((-9999 > fTemp) ? -9999 : fTemp);
                    flage_min_Height = (minTemp > fTemp) ? iWriteHeight : flage_min_Height;
                    flage_min_Width = (minTemp > fTemp) ? iWriteWidth : flage_min_Width;
                    flage_max_Height = maxTemp > fTemp ? flage_max_Height : iWriteHeight;
                    flage_max_Width = maxTemp > fTemp ? flage_max_Width : iWriteWidth;
                    minTemp = (minTemp > fTemp) ? fTemp : minTemp;
                    maxTemp = (maxTemp > fTemp) ? maxTemp : fTemp;
                }
                memset(temp, 0, sizeof(temp));
                sprintf(temp, "%.4f,", fTemp);

                if (1 != fwrite(temp, sizeof(temp), 1, fp))
                {
                    fclose(fp);
                    fp = NULL;
                    return;
                }
                iIndex += 4;
                position++;
            }
            if (1 != fwrite("\n", sizeof("\n"), 1, fp))
            {
                fclose(fp);
                fp = NULL;
                return;
            }
        }

        std::cout << "最高温度坐标:" << flage_max_Width + 1 << "," << flage_max_Height + 1 << std::endl;
        std::cout << "最高温度:" << maxTemp << std::endl;
        std::cout << "最低温度坐标:" << flage_min_Width + 1 << "," << flage_min_Height + 1 << std::endl;
        std::cout << "最低温度:" << minTemp << std::endl;
        std::cout << "最高温度图像坐标:" << (flage_max_Width) / 256.0 * (float)(pictureWidth) << "," << (flage_max_Height) / 192.0 * (float)(pictureHeight) << std::endl;
        std::cout << "最低温度图像坐标:" << (flage_min_Width) / 256.0 * (float)(pictureWidth) << "," << (flage_min_Height) / 192.0 * (float)(pictureHeight) << std::endl;

        if (datafilefp != NULL)
        {
            fclose(datafilefp);
            datafilefp = NULL;
        }
        if (fp != NULL)
        {
            fclose(fp);
            fp = NULL;
        }
        /*out.close();*/
    }
    else {
        std::cout << "设备不支持全屏测温" << std::endl;
    }
}

void ISAPI::PTZ_position3D(std::string Channel, Position position) {
    if (PTZ_Getcapabilities(capabilities_Url["云台能力获取"], "isSupportPosition3D")) {
        std::string request;
        std::string strResponseData = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                      "<position3D  xmlns=\"http://www.isapi.org/ver20/XMLSchema\" version=\"2.0\">\n"
                                        "<StartPoint>"
                                            "<positionX>" + position.StartX + "</positionX>"
                                            "<positionY>" + position.StartY + "</positionY>"
                                        "</StartPoint>\n"
                                        "<EndPoint>"
                                            "<positionX>" + position.EndX + "</positionX>"
                                            "<positionY>" + position.EndY + "</positionY>"
                                        "</EndPoint>\n"
                                      "</position3D>\n";
        std::string url = "http://" + ip + "/ISAPI/PTZCtrl/channels/" + Channel + "/position3D";
        request = GET_XML(url, "PUT", strResponseData);
        std::cout << request << std::endl;
    }
    else {
        std::cout << "该设备不支持3D定位功能" << std::endl;
    }
}

void ISAPI::PTZ_ZoomChange(std::string Channel, std::string absoluteZoom) {
    std::string resoult;
    std::string strData;
    resoult = GET_XML(capabilities_Url["绝对位置获取"], "GET", strData);
    std::cout << resoult << std::endl;
    std::string elevation_str = resoult.substr(resoult.find("<elevation>") + std::string("<elevation>").length(),
                                               resoult.find("</elevation>") - (resoult.find("<elevation>") + std::string("<elevation>").length()));
    std::string azimuth_str = resoult.substr(resoult.find("<azimuth>") + std::string("<azimuth>").length(),
                                                    resoult.find("</azimuth>") - (resoult.find("<azimuth>") + std::string("<azimuth>").length()));
    std::stringstream s_elevation, s_azimuth;
    s_elevation << atoi(elevation_str.c_str()) * 10;
    s_azimuth << atoi(azimuth_str.c_str()) * 10;
    std::string request;
    std::string strResponseData = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                  "<PTZData xmlns=\"http://www.isapi.org/ver20/XMLSchema\" version=\"2.0\">\n"
                                    "<AbsoluteHigh>\n" 
                                        /*"<!--opt, int, 海拔, range:[-900,2700]-->\n"*/
                                        "<elevation>"
                                            + s_elevation.str() +
                                        "</elevation>\n"
                                        /*"< !--opt, int, 方位角, range: [0, 3600] -->\n"*/
                                        "<azimuth>"
                                            + s_azimuth.str() +
                                        "</azimuth>\n"
                                        /*"<!--opt, int, 绝对缩放, range:[0,1000]-->\n"*/
                                        "<absoluteZoom>"
                                            + absoluteZoom +
                                        "</absoluteZoom>\n"
                                        /*"<!--opt, int, 绝对旋转角度, range:[0,350]-->"
                                        "<absoluteRotate>"
                                            "1"
                                        "</absoluteRotate>"*/
                                    "</AbsoluteHigh>\n"
                                  "</PTZData>\n";
    std::string url = "http://" + ip + "/ISAPI/PTZCtrl/channels/" + Channel + "/absolute";
    request = GET_XML(url, "PUT", strResponseData);
    std::cout << strResponseData << std::endl;
    std::cout  << std::endl;
    std::cout << request << std::endl;
}

void ISAPI::PTZ_Absolute(std::string Channel,std::string pan, std::string tilt, std::string Zoom) {
    std::string request;
    std::string strResponseData = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                  "<PTZData xmlns=\"http://www.isapi.org/ver20/XMLSchema\" version=\"2.0\">\n"
                                    "<AbsoluteHigh>\n" 
                                        /*"<!--opt, int, 海拔, range:[-900,2700]-->\n"*/
                                        "<elevation>"
                                            + tilt +
                                        "</elevation>\n"
                                        /*"< !--opt, int, 方位角, range: [0, 3600] -->\n"*/
                                        "<azimuth>"
                                            + pan +
                                        "</azimuth>\n"
                                        /*"<!--opt, int, 绝对缩放, range:[0,1000]-->\n"*/
                                        "<absoluteZoom>"
                                            + Zoom +
                                        "</absoluteZoom>\n"
                                        /*"<!--opt, int, 绝对旋转角度, range:[0,350]-->"
                                        "<absoluteRotate>"
                                            "1"
                                        "</absoluteRotate>"*/
                                    "</AbsoluteHigh>\n"
                                  "</PTZData>\n";
    std::string url = "http://" + ip + "/ISAPI/PTZCtrl/channels/" + Channel + "/absolute";
    request = GET_XML(url, "PUT", strResponseData);
    std::cout << strResponseData << std::endl;
    std::cout  << std::endl;
    std::cout << request << std::endl;
}

void ISAPI::PTZ_Proportionalpan(std::string url){
    std::string resoult;
    std::string strData;
    resoult = GET_XML(url, "GET", strData);
    std::cout << resoult << std::endl;
}

void ISAPI::PTZ_Deletepatrols(std::string url){
    std::string resoult;
    std::string strData;
    resoult = GET_XML(url, "GET", strData);
    std::cout << resoult << std::endl;
}

void ISAPI::PTZ_gotoPosition(std::string Channel){
    std::string resoult;
    std::string strData;
    std::string url = "http://" + ip + "/ISAPI/PTZCtrl/channels/" + Channel + "/gotoPosition?format=json";
    resoult = GET_XML(url, "PUT", strData);
    std::cout << resoult << std::endl;
}

void ISAPI::PTZ_GETLFPosition(std::string Channel){
    std::string resoult;
    std::string strData;
    std::string url = "http://" + ip + "/ISAPI/PTZCtrl/channels/" + Channel + "/LFPosition";
    resoult = GET_XML(url, "GET", strData);
    std::cout << resoult << std::endl;
}

void ISAPI::PTZ_PUTLFPosition(std::string Channel){
    std::string resoult;
    std::string strData;
    std::string url = "http://" + ip + "/ISAPI/PTZCtrl/channels/" + Channel + "/LFPosition";
    resoult = GET_XML(url, "PUT", strData);
    std::cout << resoult << std::endl;
}

void ISAPI::PTZ_PUThomeposition(std::string Channel){
    std::string resoult;
    std::string strData;
    std::string url = "http://" + ip + "/ISAPI/PTZCtrl/channels/" + Channel + "/homeposition";
    resoult = GET_XML(url, "PUT", strData);
    std::cout << resoult << std::endl;
}

void ISAPI::PTZ_DELETEhomeposition(std::string Channel){
    std::string resoult;
    std::string strData;
    std::string url = "http://" + ip + "/ISAPI/PTZCtrl/channels/" + Channel + "/homeposition";
    resoult = GET_XML(url, "DELETE", strData);
    std::cout << resoult << std::endl;
}

void ISAPI::PTZ_Gotohomeposition(std::string Channel){
    std::string resoult;
    std::string strData;
    std::string url = "http://" + ip + "/ISAPI/PTZCtrl/channels/" + Channel + "/homeposition/goto";
    resoult = GET_XML(url, "PUT", strData);
    std::cout << resoult << std::endl;
}


void ISAPI::ISAPI_interface(std::string url) {
    notification("1");
    if (url == Action_Url["抓热图"]) {
        int x1, y1, x2, y2;
        std::cin >> x1 >> y1 >> x2 >> y2;
        jpegPicWithAppendData("2", x1, y1, x2, y2);
    }
    if (url == Action_Url["可见光抓图"]){
        ImageCapture("1");
    }
    if (url == Action_Url["设置预置点"]){
        std::cout << "请输入需要调用预置点:" << std::endl;
        std::string presetID;
        std::cin >> presetID;
        PTZ_PresetsGoto("1", presetID);
    }
    else if (url == Action_Url["一键聚焦参数配置"])
    {
        PTZ_OnepushfoucsStart(url);
    }
    else if (url == Action_Url["重置云台一键聚焦参数"])
    {
        PTZ_OnepushfoucsReset(Action_Url["重置云台一键聚焦参数"]);
    }
    else if (url == Action_Url["云台绝对位置运动"])
    {
        int absoluteZoom;
        std::cout << "请输入zoom倍数:";
        std::cin >> absoluteZoom;
        std::stringstream ss;
        ss << absoluteZoom * 10;
        PTZ_ZoomChange("1",ss.str());
    }
    else if (url == Action_Url["云台相对位置运动"])
    {
        int P,T,Z;
        std::cout << "请输入P:";
        std::cin >> P;
        std::cout << "请输入T:";
        std::cin >> T;
        std::cout << "请输入Z:";
        std::cin >> Z;
        PTZ_continuous("1", P, T, Z);
    }
    else if (url == Action_Url["3D定位"]){
        Position position;
        std::stringstream StartX, StartY, EndX, EndY;
        float StartPointX, StartPointY, EndPointX, EndPointY;
        std::string PIC_request;
        std::string strResponseData;
        PIC_request = GET_XML(capabilities_Url["系统能力获取"], "GET", strResponseData);
        int pictureWidth = atoi(PIC_request.substr(PIC_request.find("<pictureWidth>") + std::string("<pictureWidth>").length(),
            PIC_request.find("</pictureWidth>") - (PIC_request.find("<pictureWidth>") + std::string("<pictureWidth>").length())).c_str());
        int pictureHeight = atoi(PIC_request.substr(PIC_request.find("<pictureHeight>") + std::string("<pictureHeight>").length(),
            PIC_request.find("</pictureHeight>") - (PIC_request.find("<pictureHeight>") + std::string("<pictureHeight>").length())).c_str());
        std::cin >> StartPointX >> StartPointY >> EndPointX >> EndPointY;
        StartX << (int)(StartPointX / (float)pictureWidth * 255.0);
        position.StartX = StartX.str();
        StartY << (int)(255.0 - StartPointY / (float)pictureHeight * 255.0);
        position.StartY = StartY.str();
        EndX << (int)(EndPointX / (float)pictureWidth * 255.0);
        position.EndX = EndX.str();
        EndY << (int)(255.0 - EndPointY / (float)pictureHeight * 255.0);
        position.EndY = EndY.str();
        PTZ_position3D("1", position);
    }   
}
//目前设备不支持以下功能
void ISAPI::Get_ProfileId(){
    std::string strResponseData;
    std::string request = GET_XML(capabilities_Url["获取通道号"], "GET", strResponseData);
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << request << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    std::string id = request.substr(request.find("<id>") + std::string("<id>").length(),
            request.find("</id>") - (request.find("<id>") + std::string("<id>").length())).c_str();
    std::cout << id << std::endl;
}

 void ISAPI::PTZ_Lock(std::string url){
    if (PTZ_Getcapabilities(capabilities_Url["云台能力获取"], "ManualPTZLockCap")){
        
    }else{
        std::cout << "设备不支持" << std::endl;
    }
 }

 void ISAPI::PTZ_Save(std::string url){
    if (PTZ_Getcapabilities(capabilities_Url["云台能力获取"], "isSupportPTZSave")){
        
    }else{
        std::cout << "设备不支持" << std::endl;
    }
 }

 void ISAPI::PTZ_OnepushfoucsStart(std::string url){
    if (PTZ_Getcapabilities(capabilities_Url["云台能力获取"], "isSupportOnepushfoucsStart")){
        std::string request;
        std::string strResponseData;
        request = GET_XML(url, "PUT", strResponseData);
        std::cout << request << std::endl;
    }else{
        std::cout << "设备不支持" << std::endl;
    }
 }

 void ISAPI::PTZ_OnepushfoucsReset(std::string url){
    if (PTZ_Getcapabilities(capabilities_Url["云台能力获取"], "isSupportOnepushfoucsReset")){
        std::string request;
        std::string strResponseData;
        request = GET_XML(url, "PUT", strResponseData);
        std::cout << request << std::endl;
    }else{
        std::cout << "设备不支持" << std::endl;
    }
 }

 void ISAPI::CreateFDLib(std::string url){
    if (PTZ_Getcapabilities(capabilities_Url["获取人脸比对能力"], "FDLibCap.CreateFDLibList.CreateFDLib.autoUpdata")){

    }else{
        std::cout << "设备不支持" << std::endl;
    }
 }

 void ISAPI::scheduledCapture(std::string url){
    std::string request;
        std::string strResponseData;
        request = GET_XML(capabilities_Url["获取实时流定时抓图任务能力集"], "GET", strResponseData);
        std::cout << request << std::endl;
}

 void ISAPI::SearchByTargetType(std::string url){
    std::string request;
    std::string strResponseData;
    request = GET_XML(capabilities_Url["获取设备支持录像查询能力"], "GET", strResponseData);
    std::cout << request << std::endl;
 }

 void ISAPI::auxcontrols(std::string url){
    if (PTZ_Getcapabilities(capabilities_Url["云台能力获取"], "PTZAuxType")){
        std::string request;
        std::string strResponseData;
        request = GET_XML(capabilities_Url["获取云台所有辅助设备"], "GET", strResponseData);
        std::cout << request << std::endl;
    }else{
        std::cout << "设备不支持" << std::endl;
    }
 }
 
void ISAPI::reltimepreview(std::string url)
{
    std::string request;
    std::string strResponseData;
    request = RTSP(capabilities_Url["获取设备SDP信息"]);
    std::cout << request << std::endl;
}

void ISAPI::record(std::string url)
{
    std::string request;
    std::string strResponseData;
    request = GET_XML(capabilities_Url["查询月历录像"], "POST", strResponseData);
    std::cout << request << std::endl;
}

void ISAPI::RecordSearch(std::string url){
    if (PTZ_Getcapabilities(capabilities_Url["获取设备支持录像查询能力节点"], "isSupportRecordSearch")){
        std::string request;
        std::string strResponseData;
        request = GET_XML(capabilities_Url["获取云台所有辅助设备"], "GET", strResponseData);
        std::cout << request << std::endl;
    }else{
        std::cout << "设备不支持" << std::endl;
    }
}

void ISAPI::nosubscription_arming(std::string url)
{
    std::string request;
    std::string strResponseData;
    request = GET_XML(Action_Url["建立非订阅布防连接"], "GET", strResponseData);
    std::cout << request << std::endl;
}

void ISAPI::notification(std::string url){
     if (PTZ_Getcapabilities(capabilities_Url["获取监听主机参数能力"], "HttpHostNotificationCap"))
    {
        //2.全部串口控制参数获取
        
        PTZ_Getcapabilities(capabilities_Url["获取所有串口能力"], "SerialCap");
        // PTZ_Getcapabilities(capabilities_Url["系统能力获取"], "SerialCap")
    }else{
        std::cout << "设备不支持" << std::endl;
    }
}

void ISAPI::SerialPort_disposition()
{
    //1.判断设备是否支持串口控制参数配置
    if (PTZ_Getcapabilities(capabilities_Url["系统能力获取"], "SerialCap"))
    {
        //2.全部串口控制参数获取
        PTZ_Getcapabilities(capabilities_Url["获取所有串口能力"], "SerialCap");
        std::string request;
        std::string strResponseData;
        request = GET_XML("http://" + ip + "/ISAPI/System/Serial/ports/"+"1"+"/capabilities", "GET", strResponseData);
        std::cout << request << std::endl;
        
        // PTZ_Getcapabilities(capabilities_Url["系统能力获取"], "SerialCap")
    }else{
        std::cout << "设备不支持" << std::endl;
    }
}
