#!/bin/bash

# Eliminar todos os Fifos antigos
rm -f Fifos/*

# Executar o Gestor
Bin/Gestor 2>Logs/Gestor.log

