#!/bin/bash

# Script para crear apéndice LaTeX para TFG
# Combina todos los archivos .tex generados por Doxygen

echo "📚 Creando apéndice LaTeX para TFG..."

# Verificar que existe la documentación
if [ ! -d "docs/latex" ]; then
    echo "❌ Error: No existe docs/latex"
    echo "Ejecuta primero: ./generate_simple.sh"
    exit 1
fi

# Crear directorio para el apéndice
mkdir -p docs/tfg_appendix

# Crear archivo de apéndice
cat > docs/tfg_appendix/codigo_fuente_appendix.tex << 'EOF'
% Apéndice: Código Fuente del Servidor Central
% Sistema de Control de Ascensores - TFG

\appendix
\chapter{Código Fuente del Servidor Central}

\section{Introducción}

Este apéndice contiene la documentación completa del código fuente del servidor central del sistema de control de ascensores. El servidor implementa un sistema CoAP con DTLS-PSK para la gestión segura de solicitudes de ascensores.

\section{Estructuras de Datos}

EOF

# Agregar estructuras de datos
echo "📋 Agregando estructuras de datos..."

if [ -f "docs/latex/structdtls__config__t.tex" ]; then
    echo "\\subsection{Configuración DTLS}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    cat docs/latex/structdtls__config__t.tex >> docs/tfg_appendix/codigo_fuente_appendix.tex
    echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
fi

if [ -f "docs/latex/structlogging__config__t.tex" ]; then
    echo "\\subsection{Configuración de Logging}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    cat docs/latex/structlogging__config__t.tex >> docs/tfg_appendix/codigo_fuente_appendix.tex
    echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
fi

if [ -f "docs/latex/structpsk__valid__keys__t.tex" ]; then
    echo "\\subsection{Validación de Claves PSK}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    cat docs/latex/structpsk__valid__keys__t.tex >> docs/tfg_appendix/codigo_fuente_appendix.tex
    echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
fi

if [ -f "docs/latex/structserver__context__t.tex" ]; then
    echo "\\subsection{Contexto del Servidor}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    cat docs/latex/structserver__context__t.tex >> docs/tfg_appendix/codigo_fuente_appendix.tex
    echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
fi

# Agregar headers
echo "📁 Agregando headers..."

echo "\\section{Archivos de Headers}" >> docs/tfg_appendix/codigo_fuente_appendix.tex

if [ -f "docs/latex/dtls__common__config_8h.tex" ]; then
    echo "\\subsection{Configuración DTLS (dtls\\_common\\_config.h)}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    cat docs/latex/dtls__common__config_8h.tex >> docs/tfg_appendix/codigo_fuente_appendix.tex
    echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
fi

if [ -f "docs/latex/logging_8h.tex" ]; then
    echo "\\subsection{Sistema de Logging (logging.h)}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    cat docs/latex/logging_8h.tex >> docs/tfg_appendix/codigo_fuente_appendix.tex
    echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
fi

if [ -f "docs/latex/psk__validator_8h.tex" ]; then
    echo "\\subsection{Validador PSK (psk\\_validator.h)}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    cat docs/latex/psk__validator_8h.tex >> docs/tfg_appendix/codigo_fuente_appendix.tex
    echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
fi

if [ -f "docs/latex/server__functions_8h.tex" ]; then
    echo "\\subsection{Funciones del Servidor (server\\_functions.h)}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    cat docs/latex/server__functions_8h.tex >> docs/tfg_appendix/codigo_fuente_appendix.tex
    echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
fi

# Agregar implementaciones
echo "🔧 Agregando implementaciones..."

echo "\\section{Archivos de Implementación}" >> docs/tfg_appendix/codigo_fuente_appendix.tex

if [ -f "docs/latex/main_8c.tex" ]; then
    echo "\\subsection{Servidor Principal (main.c)}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    cat docs/latex/main_8c.tex >> docs/tfg_appendix/codigo_fuente_appendix.tex
    echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
fi

if [ -f "docs/latex/psk__validator_8c.tex" ]; then
    echo "\\subsection{Implementación PSK (psk\\_validator.c)}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    cat docs/latex/psk__validator_8c.tex >> docs/tfg_appendix/codigo_fuente_appendix.tex
    echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
fi

# Cerrar el documento
cat >> docs/tfg_appendix/codigo_fuente_appendix.tex << 'EOF'

\section{Descripción del Sistema}

\subsection{Arquitectura}
El servidor central implementa un sistema CoAP con DTLS-PSK que:

\begin{itemize}
    \item Escucha en puerto 5684 (estándar CoAP-DTLS)
    \item Utiliza autenticación PSK con claves precompartidas
    \item Procesa solicitudes de ascensores desde API Gateways
    \item Implementa algoritmos de asignación inteligente
    \item Genera respuestas JSON estructuradas
\end{itemize}

\subsection{Endpoints}
\begin{itemize}
    \item \texttt{POST /peticion\_piso}: Solicitudes de llamada de piso
    \item \texttt{POST /peticion\_cabina}: Solicitudes de cabina
\end{itemize}

\subsection{Seguridad}
\begin{itemize}
    \item Autenticación DTLS-PSK
    \item Validación de identidades de clientes
    \item Cifrado de todas las comunicaciones
    \item Gestión de sesiones con timeouts optimizados
\end{itemize}

\subsection{Algoritmo de Asignación}
El sistema utiliza un algoritmo de proximidad inteligente que:
\begin{itemize}
    \item Filtra ascensores disponibles
    \item Calcula distancia absoluta desde piso de origen
    \item Selecciona ascensores con distancia mínima
    \item Resuelve empates aleatoriamente para distribuir carga
\end{itemize}

\end{document}
EOF

echo "✅ Apéndice creado: docs/tfg_appendix/codigo_fuente_appendix.tex"

# Crear también un archivo de ejemplo de inclusión
cat > docs/tfg_appendix/ejemplo_inclusion.tex << 'EOF'
% Ejemplo de cómo incluir el apéndice en tu TFG
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

% Incluir el apéndice
\include{codigo_fuente_appendix}

\end{document}
EOF

echo "📄 Ejemplo de inclusión creado: docs/tfg_appendix/ejemplo_inclusion.tex"

echo ""
echo "📊 Apéndice para TFG creado:"
echo "  📄 Apéndice: docs/tfg_appendix/codigo_fuente_appendix.tex"
echo "  📋 Ejemplo: docs/tfg_appendix/ejemplo_inclusion.tex"
echo ""
echo "💡 Para usar en tu TFG:"
echo "  1. Copia codigo_fuente_appendix.tex a tu directorio"
echo "  2. Incluye en tu documento: \\include{codigo_fuente_appendix}"
echo "  3. O copia las secciones que necesites"

echo ""
echo "✅ Proceso completado" 