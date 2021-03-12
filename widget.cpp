#include "widget.h"
#include "ui_widget.h"

#include <Windows.h>
#include <SetupAPI.h>
#include <tchar.h>
#include <stdio.h>

/*************************************
* BOOL EnumAllDevice( )
* 功能    列举当前存在的设备
* 返回值   是否成功
**************************************/
BOOL Widget::EnumAllDevice()
{
    HDEVINFO hDevInfo;
    //包含了设备的ClassGUID和设备实例句柄，大体上相当于设备信息元素(无device interface)
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD i;

    printf("Displaying the Installed Devices\n\n");
    TCHAR szGUID[] = _T("4D36E97C-E325-11CE-BFC1-08002BE10318");
    GUID guid;                  //全局唯一标识符(GUID，Globally Unique Identifier)
    UuidFromString((RPC_WSTR)szGUID, &guid);

    //创建一个设备信息集(device information set),其中包含了我们请求的设备信息元素(device information element)
    //该方法只用于获取本机设备，远程计算机上的设备需要使用SetupDiGetClassDevsEx
    //成功则返回一个设备信息集的Handle，失败则返回INVALID_HANDLE_VALUE
    hDevInfo = SetupDiGetClassDevs(
    &guid,  // 指定一个GUID(for a device setup class or a device interface class)用以过滤元素
    0,      // Enumerator 可为NULL
    0,      // hwndParent 可为NULL
    DIGCF_PRESENT | DIGCF_ALLCLASSES ); //列表的过滤选项，当为DIGCF_ALLCLASSES时，参数1指定的guid将不再起作用

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    // 循环列举
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    //枚举设备信息列表中的元素，将元素以SP_DEVINFO_DATA结构体的方式输出
    for (i=0;SetupDiEnumDeviceInfo(hDevInfo,i, &DeviceInfoData);i++)
    {
        DWORD DataT;
        LPTSTR buffer = NULL;
        DWORD buffersize = 0;

        // 获取详细信息
        while (!SetupDiGetDeviceRegistryProperty(
            hDevInfo,
            &DeviceInfoData,
            SPDRP_DEVICEDESC,   //属性标识，用以指示buffer输出的是哪个属性值，在此是要求输出设备描述字符串
            &DataT,
            (PBYTE)buffer,      //输出值
            buffersize,
            &buffersize))       //所需的buff空间
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                // 内存不足，则重新分配buff空间
                if (buffer)
                    HeapFree(GetProcessHeap(), 0, buffer);

                buffer = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, buffersize);
            }
            else
                break;
        }
        QString name =  QString::fromUtf16((const ushort*)buffer);
        ui->listWidget_DeviceList->insertItem(0, name);
        // 输出
        wprintf(L"GUID:{%.8X-%.4X-%.4X--%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X} \tDevice: %s\n",
            DeviceInfoData.ClassGuid.Data1,
            DeviceInfoData.ClassGuid.Data2,
            DeviceInfoData.ClassGuid.Data3,
            DeviceInfoData.ClassGuid.Data4[0],
            DeviceInfoData.ClassGuid.Data4[1],
            DeviceInfoData.ClassGuid.Data4[2],
            DeviceInfoData.ClassGuid.Data4[3],
            DeviceInfoData.ClassGuid.Data4[4],
            DeviceInfoData.ClassGuid.Data4[5],
            DeviceInfoData.ClassGuid.Data4[6],
            DeviceInfoData.ClassGuid.Data4[7],
            buffer);

        if (buffer) HeapFree(GetProcessHeap(), 0, buffer);
    }

    if ( GetLastError() != NO_ERROR
        && GetLastError() != ERROR_NO_MORE_ITEMS )
    {
        return FALSE;
    }
    // 释放
    SetupDiDestroyDeviceInfoList(hDevInfo);
    return TRUE;
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_EnumDevices_clicked()
{
    EnumAllDevice();
}
