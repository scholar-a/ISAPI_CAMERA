#pragma once
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iterator>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <curl/curl.h>
#include <iomanip>
#include "INIParser.h"

struct Preset{
    std::string id;
    std::string presetName;
};

struct Position{
    std::string StartX;
    std::string StartY;
    std::string EndX;
    std::string EndY;
};

class ISAPI
{
public:
    ISAPI();
    void ISAPI_interface(std::string url);
    bool PTZ_Getcapabilities(std::string url, std::string isSupport);//获取设备能力

    //云台控制
    //---------------设备不支持--------------------
    void auxcontrols(std::string url);//辅助设备控制
    void PTZ_Lock(std::string url);//云台锁定
    void PTZ_Save(std::string url);//云台位置保存
    //---------------设备可用----------------------
    void ImageCapture(std::string Channel);//可见光抓图
    void PTZ_OnepushfoucsReset(std::string url);//重置聚焦参数
    void PTZ_OnepushfoucsStart(std::string url);//云台一键聚焦
    void PTZ_AbsoluteContorl(std::string url);//绝对位置控制
    void PTZ_position3D(std::string Channel, Position position);//3D定位
    void PTZ_ZoomChange(std::string Channel, std::string absoluteZoom);//PTZ
    void PTZ_GetAbsoluteContorl(std::string url);//获取绝对位置
    void PTZ_continuous(std::string url, int P = 0, int T = 0, int Z = 0, int R = 0);//PTZ控制 pan、tilt、zoom、rotate的取值大小表示转动或变化的速度大小，取值正负表示转动或变化的方向
    void PTZ_POSTPersets(std::string url, std::string enabled, std::string id, std::string presetName);
    void PTZ_PresetsGoto(std::string Channel, std::string presetID);
    void PTZ_Absolute(std::string Channel,std::string pan, std::string tail, std::string Zoom);
    void PTZ_Proportionalpan(std::string url);
    void PTZ_gotoPosition(std::string Channel);//PTZ位置恢复
    void PTZ_GETLFPosition(std::string Channel);
    void PTZ_PUTLFPosition(std::string Channel);
    void PTZ_PUThomeposition(std::string Channel);
    void PTZ_DELETEhomeposition(std::string Channel);
    void PTZ_Gotohomeposition(std::string Channel);
    

    void PTZ_Deletepatrols(std::string url);
    //红外测温
    void realTimethermometry(std::string url);//实时测温
    //使用默认参数,进行全屏测温抓图
	void jpegPicWithAppendData(std::string ThermalChannel, int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0);//全屏测温 

    //录像
    //暂时不写,设备不支持
    void record(std::string url);//录像回放
    void RecordSearch(std::string url);//查询录像&图片
    void reltimepreview(std::string url);//实时预览
    void nosubscription_arming(std::string url);//非订阅布防
    void SerialPort_disposition();
    void scheduledCapture(std::string url);//定时抓图
    void SearchByTargetType(std::string url);//按目标类型检测录像
    void notification(std::string url);//警报监听
   
    //车辆识别
    
    //人脸识别
    void CreateFDLib(std::string url);//创建人脸聚类库
    
    void Get_ProfileId();
public:
    std::map<std::string, std::string> Action_Url;
    std::vector<Preset> presets;
private:
    void GetChannals(std::string ip);
    void PTZ_Getpresets(std::string url);//获取预置点
    void PTZ_PresetsMenu();
    std::string GET_XML(std::string url, std::string modth, std::string strResponseData);//获取回复XML信息
    std::string RTSP(std::string url);//
    void readconfigFile(void)
    {
        ini_parser.ReadINI("./Config/ISAPI_Config.ini");
        ip=ini_parser.GetValue("camera", "camIP").c_str();
        PTZChannel=ini_parser.GetValue("camera", "PTZChannel").c_str();
        ThermalChannel=ini_parser.GetValue("camera", "ThermalChannel").c_str();
        username = ini_parser.GetValue("camera", "username").c_str();
        password = ini_parser.GetValue("camera", "password").c_str();
    }
private:
    INIParser ini_parser;
    std::string username;
    std::string password;
    std::string ThermalChannel;
    std::string PTZChannel;
    std::string ip;
    std::map<std::string, std::string> capabilities_Url; 
};