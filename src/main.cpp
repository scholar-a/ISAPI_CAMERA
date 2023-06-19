#include "ISAPI.h"

int main(int argc, char* argv[])
{
    ISAPI access; 
    /*access.ISAPI_interface(access.key[3]);*/
    while(1){
        int opt;
        std::cout << "请输入选项：" << std::endl;
        std::cin >> opt;
        switch(opt){
            case 1:
                access.ISAPI_interface(access.Action_Url["抓热图"]);
                break;
            case 2:
                access.ISAPI_interface(access.Action_Url["可见光抓图"]);
                break;
            case 3:
                access.ISAPI_interface(access.Action_Url["云台绝对位置运动"]);
                break;
            case 4:
                access.ISAPI_interface(access.Action_Url["设置预置点"]);
                break;
            case 5:
                access.ISAPI_interface(access.Action_Url["3D定位"]);
                break;
            case 6:
                access.ISAPI_interface(access.Action_Url["云台相对位置运动"]);
                break;
            case 7:
                access.ISAPI_interface(access.Action_Url["一键聚焦参数配置"]);
                break;
            case 8:
                access.ISAPI_interface(access.Action_Url["重置云台一键聚焦参数"]);
                break;

            default:
                std::cout << "非法输入" << std::endl;
                break;
        }
    }
    
    return 0;
}