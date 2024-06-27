R3_REGISTRATION_CODE="3571692A-EB69-5189-ACB2-7D80EB803363" sh -c "$(curl -L https://downloads.remote.it/remoteit/install_agent.sh)"
https://www.tomshardware.com/how-to/fix-cannot-currently-show-desktop-error-raspberry-pi

  
import gpiozero
from time import sleep

# Inisialisasi objek untuk motor dan tombol
button = gpiozero.Button(17)  # GPIO17 terhubung ke tombol
Motor1A = gpiozero.OutputDevice(8)  # GPIO8 untuk Motor1A
Motor1B = gpiozero.OutputDevice(7)  # GPIO7 untuk Motor1B
Motor1E = gpiozero.PWMOutputDevice(25)  # GPIO25 untuk Motor1E (PWM)

def loop():
    while True:  
        if button.is_pressed:
            Motor1A.off()
            Motor1B.on()
            Motor1E.value = 1.0  # Kecepatan maksimum (1.0)
        else:
           Motor1A.on()
           Motor1B.off()
           Motor1E.value = 1.0  # Kecepatan maksimum (1.0) 
            

def destroy():
    Motor1E.value = 0  # Matikan motor sebelum keluar
    Motor1A.close()
    Motor1B.close()
    Motor1E.close()

if __name__ == '__main__':
    try:
        loop()
    except KeyboardInterrupt:
        destroy()
        print("Program Stopped")
