# team Edge's logic is very smart, i am working on top of it.

from machine import UART
import sensor,  image, lcd,  time
from fpioa_manager import fm
from Maix import GPIO
import struct
fm.register(34, fm.fpioa.UART1_TX)
fm.register(35,fm.fpioa.UART1_RX)

fm.register(18, fm.fpioa.GPIO1)
ButtonA=GPIO(GPIO.GPIO1, GPIO.IN, GPIO.PULL_UP)
fm.register(19, fm.fpioa.GPIO2)
ButtonB=GPIO(GPIO.GPIO2, GPIO.IN, GPIO.PULL_UP)

#uart = UART(UART.UART1, 115200,8,0,0, timeout=1000, read_buf_len=4096)
uart = UART(UART.UART1, 115200, 8, None, 1, timeout=1000, read_buf_len=4096)

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QQVGA)
sensor.set_windowing((160,120))
sensor.set_brightness(3)
sensor.set_contrast(3)
sensor.run(1)
#blue_threshold   = [(0, 100, -127, 125, -78, -37)]
blue_threshold = [(21, 97, -128, 127, -128, -23)]
yellow_threshold = [(21, 97, -30, 20, 39, 111)]


Y_th = yellow_threshold[0]
B_th = blue_threshold[0]
GAIN = 23.0
WHITE_GAIN = (90.0, 56.0, 115.0)

confidencial = 0.25 #0.25

myroi = (0,0,160,120) # (0, 120, 320, 100)
senter = (150,0,20,240)
senter_flag = 0

x_b = -1;
x_y = -1;
clock = time.clock()







print("gain = ",end="")
print(sensor.get_gain_db())
print(" WHITE_BAL = ",end="")
print(sensor.get_rgb_gain_db())

sensor.set_auto_gain(False,gain_gb = GAIN,gain_db_ceiling = GAIN)
sensor.set_auto_whitebal(False,WHITE_GAIN)

#blobs型の配列 0:x 1:y 2:w 3:h 4:n 5:x 6:y (Array of type blobs 0:x 1:y 2:w 3:h 4:n 5:x 6:y)
while True:
    senter_flag = 0
    clock.tick()
    img = sensor.snapshot()
    img.rotation_corr(z_rotation=-90)
    send_list = [255,0,0,0,0,0,0,254] #7こ

    if img:     #画像がなかったらエラー起こるからね、しょうがないね (An error will occur if there is no image, so it can't be helped.)
        try:    #よくfind_blobsで例外起こるからｔｒｙ文に入れてる (I often get exceptions with `find_blobs`, so I put it in a `try` statement.)
            blobs_blue = img.find_blobs(blue_threshold,roi = myroi,pixels_threshold = 100)
            blobs_yellow = img.find_blobs(yellow_threshold,roi = myroi,pixels_threshold = 100)
            if img.find_blobs(yellow_threshold,roi = senter) or img.find_blobs(blue_threshold,roi = senter):
                senter_flag = 1     #画像の中心にブロックがあったらシュートできるよ (If there's a block in the center of the image, you can shoot.)

        except AttributeError as E: #特に意味ない (It doesn't have any particular meaning.)
            print(E)

        if blobs_blue:      #こっから青色判定ゾーン (From here on, it's the blue judgment zone.)
            max_area_b = 0  #ブロックの面積の最大値初期化 (Initialization of the maximum block area)
            target_b=blobs_blue[0]  #変数の初期化 (Variable initialization)
            for b in blobs_blue:    #for文で回す (Loop using a for loop)
                if b.area() > max_area_b:   #面積を比較してもっともデカかったら記録 (Compare the areas and record the largest one.)
                    max_area_b = b.area()
                    target_b = b

            tmp=img.draw_rectangle(target_b[0:4])   #ブロックを枠でかこう (Enclose the block with a frame.)
            width_b = target_b[2] / 8   #敵避け用にブロックの幅を8で割る (Divide the width of the block by 8 to avoid enemies.)
            height_b = target_b[3] / 20 #敵避け用にブロックの高さを8で割る (Divide the block height by 8 to avoid enemies.)
            H = 0       #ブロックごとの一番下にある青色のピクセルのある高さを記録 (Record the height of the blue pixel at the bottom of each block.)
            H_old = 0   #見てるブロックの一つ左の高さ (The height of the block to the left of the one you are looking at)
            Flag = 0    #敵感知した? (Enemy detected?)
            robot_range=[0,8]
            x_b=target_b[5] #ブロックの中心を座標として記録 (Record the center of the block as its coordinates.)

            for i in range(0,8):    #こっから敵感知(ここは横向きに分けてる) (Enemy detection starts from here (this area is divided horizontally).)
                X = target_b[0]+width_b*i   #ブロック分けしたx座標 (x coordinates divided into blocks)
                for j in range(1,20):       #高さでfor文回してる (The for loop is iterating through the height.)
                    Y=target_b[1]+target_b[3]-height_b*j    #高さの変数 (Height variable)
                    pixel=img.get_pixel(int(X),int(Y))      #色を入手 (Get the color)
                    pixel=image.rgb_to_lab(pixel)           #LABに色変換 (Color conversion to LAB)

                    if B_th[4]<pixel[2] and pixel[2]<B_th[5]:   #Bの閾値におさまってるか判定 (Determine if it falls within threshold B.)
                        tmp=img.draw_cross(int(X),int(Y),color=(0,0,200))
                        H = j           #一番したにある高さを記録 (Record the lowest height.)
                        if 2<H - H_old: #隣り合ったピクセルの高さの差を見てるよ ある程度差があったら敵と検知 (It's looking at the height difference between adjacent pixels. If there's a certain difference, it detects it as an enemy.)
                            Flag=1
                            robot_range[0]=i
                        if H - H_old<-3:
                            Flag=1
                            robot_range[1]=i

                        H_old=H
                        break


            if Flag!=0:
                if 5<abs(robot_range[0]-robot_range[1]):    #あんまり敵の幅が大きい時はおかしいから無視する (If the enemy's width is too wide, it's strange, so ignore it.)
                    Flag=0

            range_s=int(target_b[0]+width_b*robot_range[0]) #敵がいるx座標の始まり (The start of the x-coordinate where the enemy is located.)
            range_f=int(target_b[0]+width_b*robot_range[1]) #敵がいるx座標の終わり (The end of the x-coordinate where the enemy is located.)

            if Flag!=0:     #敵がいたと検知されたら (If an enemy is detected)
                tmp=img.draw_line(range_s,target_b[1],range_f,target_b[1],color=(0,0,200),thickness=10)
                if 8-robot_range[1]<robot_range[0]:     #なんかいろいろ都合のいいようにするやつ(何かいてるかわからん) (Someone who does things in a way that suits their own convenience (I don't know what they're writing))
                    x_b=int((1 - confidencial) * target_b[0] + confidencial * range_s)
                else:
                    x_b=int((1 - confidencial) * (target_b[0] + target_b[2]) + confidencial * range_f)
                    #print(x_b)

                if (100<range_s or range_f<140)or(range_s<100 and 140<range_f):
                    senter_flag=0


            tmp=img.draw_cross(int(x_b),int(target_b[6]),color=(200,0,0),size=10)
            #x_b /= 2    #1バイトにおさまるように2で割ってる (Divided by 2 to fit into 1 byte.)
            #send_list[1] = int('e')
            #send_list[2] = int(x_b)
            #send_list[3] = int(target_b[0] / 2.0)
            #send_list[4] = int(target_b[1])
            #send_list[5] = int(target_b[2] / 2.0)
            #send_list[6] = int(target_b[3])
            #send = bytearray(send_list)
            #uart.write(send)

        #else:
            #send_list[1] = int(0)
            #send_list[2] = int(0)
            #send_list[3] = int(0)
            #send_list[4] = int(0)
            #send_list[5] = int(0)
            #send_list[6] = int(0)
            #send = bytearray(send_list)
            #uart.write(send)

        print(send_list)

        if blobs_yellow:    #青色と同じなので割愛 (It's the same as blue, so I'll omit it.)
            target_w=blobs_yellow[0]
            max_area_w = 0
            for b in blobs_yellow:
                if b.area() > max_area_w:
                    max_area_w = b.area()
                    target_w = b

            tmp=img.draw_rectangle(target_w[0:4])
            width_y = target_w[2] / 8
            height_y = target_w[3] / 20
            H = 0
            H_old = 0
            Flag = 0
            robot_range=[0,8]
            x_y=target_w[5]
            for i in range(1,8):
                X = target_w[0]+width_y*i
                for j in range(1,20):
                    Y=target_w[1]+target_w[3]-height_y*j
                    pixel=img.get_pixel(int(X),int(Y))
                    pixel=image.rgb_to_lab(pixel)

                    if Y_th[4]<pixel[2] and pixel[2]<Y_th[5]:
                        tmp=img.draw_cross(int(X),int(Y),color=(0,0,200))
                        H = j
                        if i == 1:
                            H_old = H

                        if 3<H - H_old:
                            Flag+=1
                            robot_range[0]=i
                        if H - H_old<-3:
                            Flag+=2
                            robot_range[1]=i-1

                        H_old=H
                        break

            if Flag!=0:
                if 5<abs(robot_range[0]-robot_range[1]):
                    Flag=0

            range_s=int(target_w[0]+width_y*robot_range[0])
            range_f=int(target_w[0]+width_y*robot_range[1])

            if Flag!=0:     #敵がいたと検知されたら (If an enemy is detected)
                tmp=img.draw_line(range_s,target_w[1],range_f,target_w[1],color=(0,0,200),thickness=10)
                if 8-robot_range[1]<robot_range[0]:     #なんかいろいろ都合のいいようにするやつ(何かいてるかわからん) (Someone who does things in a way that suits their own convenience (I don't know what they're writing))
                    x_y =int((1 - confidencial) * target_w[0] + confidencial * range_s)
                else:
                    x_y =int((1 - confidencial) * (target_w[0] + target_w[2]) + confidencial * range_f)
                #print(x_y)

                if (100<range_s or range_f<140)or(range_s<100 and 140<range_f):
                    senter_flag=0

            tmp=img.draw_cross(int(x_y),int(target_w[6]),color=(200,0,0),size=5)
            #x_y /= 2
            #send_list[1] = int('f')
            #send_list[2] = int(x_y)
            #send_list[3] = int(target_w[0] / 2.0)
            #send_list[4] = int(target_w[1])
            #send_list[5] = int(target_w[2] / 2.0)
            #send_list[6] = int(target_w[3])
            #send = bytearray(send_list)
            #uart.write(send)
            #print(senter_flag)
        #else:
            #send_list[1] = int(1)
            #send_list[2] = int(0)
            #send_list[3] = int(0)
            #send_list[4] = int(0)
            #send_list[5] = int(0)
            #send_list[6] = int(0)
            #send = bytearray(send_list)
            #uart.write(send)
    if len(blobs_yellow) == 0:
        x_y = -1;
    if len(blobs_blue) == 0:
        x_b = -1;

    packet = struct.pack('<Bii', ord('e'), x_y, x_b)

    uart.write(packet)

    print(x_y, x_b)
