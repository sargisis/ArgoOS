#!/usr/bin/env python3
import socket
import threading
import sys
import os

# Настройки подключения к QEMU
HOST = '127.0.0.1'
PORT = 4444

def receive_data(sock):
    """Слушает входящие данные от ядра (ArgOS) и выводит их в консоль."""
    while True:
        try:
            data = sock.recv(1024)
            if not data:
                print("\n[ОШИБКА] Соединение разорвано ядром.")
                os._exit(0)
            
            # Выводим полученный текст (включая сырую телеметрию)
            text = data.decode('utf-8', errors='replace')
            text = text.replace('\n', '\r\n')
            sys.stdout.write(text)
            sys.stdout.flush()
        except Exception as e:
            print(f"\n[ОШИБКА] При чтении: {e}")
            os._exit(1)

def main():
    print(f"[*] Подключение к ArgOS по адресу {HOST}:{PORT}...")
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))
        print("[*] Успешно подключено! Мост готов.\n")
    except ConnectionRefusedError:
        print(f"[ОШИБКА] Не удалось подключиться к {HOST}:{PORT}.")
        print("Убедитесь, что ядро запущено командой: make qemu-bridge")
        sys.exit(1)

    # Запускаем поток для чтения данных из ядра
    t = threading.Thread(target=receive_data, args=(sock,), daemon=True)
    t.start()

    # Основной поток: читаем ввод пользователя и отправляем в ядро
    try:
        while True:
            # Читаем символ за символом
            char = sys.stdin.read(1)
            if char:
                sock.sendall(char.encode('utf-8'))
    except KeyboardInterrupt:
        print("\n[*] Завершение работы моста...")
    finally:
        sock.close()

if __name__ == "__main__":
    # Настраиваем stdin для чтения по одному символу без буферизации
    try:
        import termios
        import tty
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        tty.setraw(sys.stdin.fileno())
    except ImportError:
        pass # На Windows это не сработает, но ОС работает в Linux

    try:
        main()
    finally:
        # Восстанавливаем настройки терминала
        if 'old_settings' in locals():
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
