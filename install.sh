#!/bin/bash

# Обновление списков пакетов
echo "Updating package lists..."
sudo apt update -y

# Установка системных зависимостей через apt
echo "Installing system dependencies..."
sudo apt-get install -y qt5-default
sudo apt-get install -y qtbase5-dev
sudo apt-get install -y libboost-all-dev
sudo apt install -y libpqxx-dev
sudo apt-get install -y libsodium-dev
sudo apt install -y postgresql-client

# Установка Python-библиотеки через pip
echo "Installing Python dependencies..."
pip install yoyo-migrations

# Печать завершения установки
echo "Installation completed!"
