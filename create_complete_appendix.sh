#!/bin/bash

# Script para crear apéndice completo con Servidor Central + API Gateway
# Sistema de Control de Ascensores - TFG

echo "📚 Creando apéndice completo con Servidor Central + API Gateway..."

# Verificar que existen las documentaciones
if [ ! -d "servidor_central/docs/latex" ]; then
    echo "❌ Error: No existe documentación del servidor central"
    echo "Ejecuta primero: cd servidor_central && ./generate_simple.sh"
    exit 1
fi

if [ ! -d "api_gateway/docs/latex" ]; then
    echo "❌ Error: No existe documentación del API Gateway"
    echo "Ejecuta primero: cd api_gateway && ./generate_docs.sh"
    exit 1
fi

# Crear directorio para el apéndice completo
mkdir -p docs/complete_appendix

# Crear archivo de apéndice completo
cat > docs/complete_appendix/codigo_fuente_completo.tex << 'EOF'
% Apéndice: Código Fuente Completo del Sistema de Control de Ascensores
% Servidor Central + API Gateway - TFG

\appendix
\chapter{Código Fuente Completo del Sistema de Control de Ascensores}

\section{Introducción}

Este apéndice contiene la documentación completa del código fuente del sistema de control de ascensores, incluyendo tanto el servidor central como el API Gateway. El sistema implementa un arquitectura distribuida con comunicación segura CoAP/DTLS-PSK.

\section{Servidor Central}

El servidor central implementa un sistema CoAP con DTLS-PSK que gestiona la asignación de tareas a ascensores en el sistema distribuido.

EOF

# Agregar documentación del servidor central
echo "📋 Agregando documentación del servidor central..."

# Agregar estructuras de datos del servidor central
if [ -f "servidor_central/docs/latex/structdtls__config__t.tex" ]; then
    echo "\\subsection{Configuración DTLS (Servidor Central)}" >> docs/complete_appendix/codigo_fuente_completo.tex
    cat servidor_central/docs/latex/structdtls__config__t.tex >> docs/complete_appendix/codigo_fuente_completo.tex
    echo "" >> docs/complete_appendix/codigo_fuente_completo.tex
fi

if [ -f "servidor_central/docs/latex/structlogging__config__t.tex" ]; then
    echo "\\subsection{Configuración de Logging (Servidor Central)}" >> docs/complete_appendix/codigo_fuente_completo.tex
    cat servidor_central/docs/latex/structlogging__config__t.tex >> docs/complete_appendix/codigo_fuente_completo.tex
    echo "" >> docs/complete_appendix/codigo_fuente_completo.tex
fi

if [ -f "servidor_central/docs/latex/structpsk__valid__keys__t.tex" ]; then
    echo "\\subsection{Validación de Claves PSK (Servidor Central)}" >> docs/complete_appendix/codigo_fuente_completo.tex
    cat servidor_central/docs/latex/structpsk__valid__keys__t.tex >> docs/complete_appendix/codigo_fuente_completo.tex
    echo "" >> docs/complete_appendix/codigo_fuente_completo.tex
fi

if [ -f "servidor_central/docs/latex/structserver__context__t.tex" ]; then
    echo "\\subsection{Contexto del Servidor (Servidor Central)}" >> docs/complete_appendix/codigo_fuente_completo.tex
    cat servidor_central/docs/latex/structserver__context__t.tex >> docs/complete_appendix/codigo_fuente_completo.tex
    echo "" >> docs/complete_appendix/codigo_fuente_completo.tex
fi

# Agregar headers del servidor central
echo "\\subsection{Headers del Servidor Central}" >> docs/complete_appendix/codigo_fuente_completo.tex

if [ -f "servidor_central/docs/latex/dtls__common__config_8h.tex" ]; then
    echo "\\subsubsection{Configuración DTLS (dtls\\_common\\_config.h)}" >> docs/complete_appendix/codigo_fuente_completo.tex
    cat servidor_central/docs/latex/dtls__common__config_8h.tex >> docs/complete_appendix/codigo_fuente_completo.tex
    echo "" >> docs/complete_appendix/codigo_fuente_completo.tex
fi

if [ -f "servidor_central/docs/latex/logging_8h.tex" ]; then
    echo "\\subsubsection{Sistema de Logging (logging.h)}" >> docs/complete_appendix/codigo_fuente_completo.tex
    cat servidor_central/docs/latex/logging_8h.tex >> docs/complete_appendix/codigo_fuente_completo.tex
    echo "" >> docs/complete_appendix/codigo_fuente_completo.tex
fi

if [ -f "servidor_central/docs/latex/psk__validator_8h.tex" ]; then
    echo "\\subsubsection{Validador PSK (psk\\_validator.h)}" >> docs/complete_appendix/codigo_fuente_completo.tex
    cat servidor_central/docs/latex/psk__validator_8h.tex >> docs/complete_appendix/codigo_fuente_completo.tex
    echo "" >> docs/complete_appendix/codigo_fuente_completo.tex
fi

if [ -f "servidor_central/docs/latex/server__functions_8h.tex" ]; then
    echo "\\subsubsection{Funciones del Servidor (server\\_functions.h)}" >> docs/complete_appendix/codigo_fuente_completo.tex
    cat servidor_central/docs/latex/server__functions_8h.tex >> docs/complete_appendix/codigo_fuente_completo.tex
    echo "" >> docs/complete_appendix/codigo_fuente_completo.tex
fi

# Agregar implementaciones del servidor central
echo "\\subsection{Implementaciones del Servidor Central}" >> docs/complete_appendix/codigo_fuente_completo.tex

if [ -f "servidor_central/docs/latex/main_8c.tex" ]; then
    echo "\\subsubsection{Servidor Principal (main.c)}" >> docs/complete_appendix/codigo_fuente_completo.tex
    cat servidor_central/docs/latex/main_8c.tex >> docs/complete_appendix/codigo_fuente_completo.tex
    echo "" >> docs/complete_appendix/codigo_fuente_completo.tex
fi

if [ -f "servidor_central/docs/latex/psk__validator_8c.tex" ]; then
    echo "\\subsubsection{Implementación PSK (psk\\_validator.c)}" >> docs/complete_appendix/codigo_fuente_completo.tex
    cat servidor_central/docs/latex/psk__validator_8c.tex >> docs/complete_appendix/codigo_fuente_completo.tex
    echo "" >> docs/complete_appendix/codigo_fuente_completo.tex
fi

# Agregar sección del API Gateway
echo "\\section{API Gateway}" >> docs/complete_appendix/codigo_fuente_completo.tex

echo "El API Gateway actúa como intermediario entre los controladores CAN de ascensores y el servidor central, implementando un puente CoAP-CAN con gestión de estado local." >> docs/complete_appendix/codigo_fuente_completo.tex

# Agregar documentación del API Gateway
echo "📋 Agregando documentación del API Gateway..."

# Buscar todos los archivos .tex del API Gateway
for tex_file in api_gateway/docs/latex/*.tex; do
    if [ -f "$tex_file" ]; then
        filename=$(basename "$tex_file" .tex)
        echo "\\subsection{$(echo $filename | sed 's/_/\\_/g')}" >> docs/complete_appendix/codigo_fuente_completo.tex
        cat "$tex_file" >> docs/complete_appendix/codigo_fuente_completo.tex
        echo "" >> docs/complete_appendix/codigo_fuente_completo.tex
    fi
done

# Cerrar el documento
cat >> docs/complete_appendix/codigo_fuente_completo.tex << 'EOF'

\section{Descripción del Sistema Completo}

\subsection{Arquitectura del Sistema}
El sistema implementa una arquitectura distribuida con:

\begin{itemize}
    \item \textbf{Servidor Central}: Gestiona asignación de tareas con algoritmos de proximidad
    \item \textbf{API Gateway}: Puente entre controladores CAN y servidor central
    \item \textbf{Comunicación Segura}: DTLS-PSK para todas las comunicaciones
    \item \textbf{Gestión de Estado}: Estado local en gateways, global en servidor
\end{itemize}

\subsection{Flujo de Comunicación}
\begin{enumerate}
    \item Ascensor envía solicitud CAN al API Gateway
    \item API Gateway convierte a CoAP y envía al servidor central
    \item Servidor central asigna ascensor óptimo
    \item Respuesta se envía de vuelta al ascensor
\end{enumerate}

\subsection{Seguridad}
\begin{itemize}
    \item Autenticación DTLS-PSK mutua
    \item Claves determinísticas basadas en identidad
    \item Validación de patrones de identidad
    \item Cifrado de todas las comunicaciones
\end{itemize}

\subsection{Algoritmos de Asignación}
\begin{itemize}
    \item \textbf{Proximidad}: Selecciona ascensor más cercano
    \item \textbf{Distribución de Carga}: Resuelve empates aleatoriamente
    \item \textbf{Optimización}: Minimiza tiempo de espera
\end{itemize}

\end{document}
EOF

echo "✅ Apéndice completo creado: docs/complete_appendix/codigo_fuente_completo.tex"

# Crear script de ejemplo de inclusión
cat > docs/complete_appendix/ejemplo_inclusion_completo.tex << 'EOF'
% Ejemplo de cómo incluir el apéndice completo en tu TFG
% Copia este contenido en tu documento principal

\documentclass[11pt,a4paper]{report}
\usepackage[utf8]{inputenc}
\usepackage[T1]{fontenc}
\usepackage{geometry}
\usepackage{graphicx}
\usepackage{listings}
\usepackage{xcolor}
\usepackage{hyperref}

% Configuración básica
\geometry{a4paper,left=2.5cm,right=2.5cm,top=2.5cm,bottom=2.5cm}

% Configuración de listados
\lstset{
    basicstyle=\ttfamily\small,
    breaklines=true,
    frame=single,
    numbers=left,
    numberstyle=\tiny,
    stepnumber=1,
    numbersep=5pt,
    backgroundcolor=\color{white},
    commentstyle=\color{green},
    keywordstyle=\color{blue},
    stringstyle=\color{red},
    showstringspaces=false,
    tabsize=4
}

\title{Tu Trabajo de Fin de Grado}
\author{Tu Nombre}
\date{\today}

\begin{document}

\maketitle
\tableofcontents

% Tu contenido principal aquí...

% Incluir el apéndice completo
\include{codigo_fuente_completo}

\end{document}
EOF

echo "📄 Ejemplo de inclusión creado: docs/complete_appendix/ejemplo_inclusion_completo.tex"

echo ""
echo "📊 Apéndice completo creado:"
echo "  📄 Apéndice: docs/complete_appendix/codigo_fuente_completo.tex"
echo "  📋 Ejemplo: docs/complete_appendix/ejemplo_inclusion_completo.tex"
echo ""
echo "💡 Para usar en tu TFG:"
echo "  1. Copia codigo_fuente_completo.tex a tu directorio"
echo "  2. Incluye en tu documento: \\include{codigo_fuente_completo}"
echo "  3. Contiene TODO: Servidor Central + API Gateway"

echo ""
echo "✅ Proceso completado" 