测试同时推送码云和GitHub
传感器： 
1、 上电初始化并进入待机状态。持续按键超过4S，系统屏蔽除第15脚（PB2）以外所有端口，完成“关机”。再次持续按键超过4S，系统初始化进入待机状态，完成“开机”。
低电压报警时，LED点亮间隔时间列表，便于修改。
2，晴天发射一次
3，上电指示

1、 YS调整电位器RT1阻值为500K，分3档调节。
电位器中心脚YS-RT（PD6）与YS-（PC4）脚阻值接近0时，YS电压感应阈值2.8V（可调）
电位器中心脚YS-RT（PD6）居于中间位置时，YS电压感应阈值为2V（可调）。
电位器中心脚YS-RT（PD6）与YS+（PD5）脚阻值接近0时，YS电压感应阈值为1.2V（可调）。
?
2、 FL调整电位器RT2阻值为500K，分3档调节。
电位器中心脚FL-RT（PB6）与FL-（PD4）脚阻值接近0时，FL启动转数阈值200转（可调）。
电位器中心脚FL--RT（PB6）居于中间位置时，FL电压感应阈值为500转（可调）。 
电位器中心脚FL-RT（PB6）与FL+（PD4）脚阻值接近0时，FL电压感应阈值为1200V。
 5、MCU第24脚（PD7）为温度检测中心脚。第10脚（PD1）正温度检测脚（TEMP+），