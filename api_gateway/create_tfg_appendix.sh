#!/bin/bash

# Script para crear apéndice LaTeX para TFG - API Gateway
# Combina todos los archivos .tex generados por Doxygen

echo "📚 Creando apéndice LaTeX para TFG - API Gateway..."

# Verificar que existe la documentación
if [ ! -d "docs/latex" ]; then
    echo "❌ Error: No existe docs/latex"
    echo "Ejecuta primero: ./generate_docs.sh"
    exit 1
fi

# Crear directorio para el apéndice
mkdir -p docs/tfg_appendix

# Crear archivo de apéndice
cat > docs/tfg_appendix/codigo_fuente_appendix.tex << 'EOF'
% Apéndice: Código Fuente del API Gateway
% Sistema de Control de Ascensores - TFG

\appendix
\chapter{Código Fuente del API Gateway}

\section{Introducción}

Este apéndice contiene la documentación completa del código fuente del API Gateway del sistema de control de ascensores. El API Gateway actúa como intermediario entre los controladores CAN de ascensores y el servidor central, implementando un puente CoAP-CAN con gestión de estado local.

\section{Estructuras de Datos}

EOF

# Buscar y agregar estructuras de datos
echo "📋 Agregando estructuras de datos..."

for struct_file in docs/latex/struct*.tex; do
    if [ -f "$struct_file" ]; then
        filename=$(basename "$struct_file" .tex)
        struct_name=$(echo $filename | sed 's/struct//' | sed 's/__/_/g')
        echo "\\subsection{Estructura $(echo $struct_name | sed 's/_/\\_/g')}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
        cat "$struct_file" >> docs/tfg_appendix/codigo_fuente_appendix.tex
        echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    fi
done

# Agregar headers
echo "📁 Agregando headers..."

echo "\\section{Archivos de Headers}" >> docs/tfg_appendix/codigo_fuente_appendix.tex

for header_file in docs/latex/*_8h.tex; do
    if [ -f "$header_file" ]; then
        filename=$(basename "$header_file" .tex)
        header_name=$(echo $filename | sed 's/_8h//' | sed 's/__/_/g')
        echo "\\subsection{$(echo $header_name | sed 's/_/\\_/g')}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
        cat "$header_file" >> docs/tfg_appendix/codigo_fuente_appendix.tex
        echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    fi
done

# Agregar implementaciones
echo "🔧 Agregando implementaciones..."

echo "\\section{Archivos de Implementación}" >> docs/tfg_appendix/codigo_fuente_appendix.tex

for impl_file in docs/latex/*_8c.tex; do
    if [ -f "$impl_file" ]; then
        filename=$(basename "$impl_file" .tex)
        impl_name=$(echo $filename | sed 's/_8c//' | sed 's/__/_/g')
        echo "\\subsection{$(echo $impl_name | sed 's/_/\\_/g')}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
        cat "$impl_file" >> docs/tfg_appendix/codigo_fuente_appendix.tex
        echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
    fi
done

# Agregar otros archivos .tex que no sean estructuras, headers o implementaciones
echo "📄 Agregando otros archivos..."

for other_file in docs/latex/*.tex; do
    if [ -f "$other_file" ]; then
        filename=$(basename "$other_file" .tex)
        # Excluir archivos ya procesados
        if [[ ! "$filename" =~ ^struct ]] && [[ ! "$filename" =~ _8h$ ]] && [[ ! "$filename" =~ _8c$ ]]; then
            echo "\\subsection{$(echo $filename | sed 's/_/\\_/g')}" >> docs/tfg_appendix/codigo_fuente_appendix.tex
            cat "$other_file" >> docs/tfg_appendix/codigo_fuente_appendix.tex
            echo "" >> docs/tfg_appendix/codigo_fuente_appendix.tex
        fi
    fi
done

# Cerrar el documento
cat >> docs/tfg_appendix/codigo_fuente_appendix.tex << 'EOF'

\section{Descripción del API Gateway}

\subsection{Arquitectura}
El API Gateway implementa un sistema CoAP con DTLS-PSK que:

\begin{itemize}
    \item Actúa como puente entre controladores CAN y servidor central
    \item Gestiona estado local de ascensores del edificio
    \item Convierte mensajes CAN a solicitudes CoAP
    \item Implementa simulación de movimiento de ascensores
    \item Mantiene sesiones DTLS con servidor central
\end{itemize}

\subsection{Funcionalidades Principales}
\begin{itemize}
    \item \textbf{Puente CAN-CoAP}: Transformación de protocolos
    \item \textbf{Gestión de Estado}: Estado local de ascensores
    \item \textbf{Simulación}: Movimiento automático de ascensores
    \item \textbf{Comunicación Segura}: DTLS-PSK con servidor central
    \item \textbf{Logging}: Sistema de registro detallado
    \item \textbf{Manejo de Señales}: Terminación elegante
\end{itemize}

\subsection{Endpoints CoAP}
\begin{itemize}
    \item \texttt{POST /peticion\_piso}: Solicitudes de llamada de piso
    \item \texttt{POST /peticion\_cabina}: Solicitudes de cabina
\end{itemize}

\subsection{Seguridad}
\begin{itemize}
    \item Autenticación DTLS-PSK mutua
    \item Claves determinísticas basadas en identidad
    \item Validación de patrones de identidad
    \item Cifrado de todas las comunicaciones
\end{itemize}

\subsection{Simulación}
El API Gateway incluye un sistema de simulación que:
\begin{itemize}
    \item Simula movimiento automático de ascensores
    \item Actualiza posiciones y estados
    \item Genera eventos de llegada a pisos
    \item Mantiene sincronización con estado real
\end{itemize}

\end{document}
EOF

echo "✅ Apéndice creado: docs/tfg_appendix/codigo_fuente_appendix.tex"

# Crear también un archivo de ejemplo de inclusión
cat > docs/tfg_appendix/ejemplo_inclusion.tex << 'EOF'
% Ejemplo de cómo incluir el apéndice del API Gateway en tu TFG
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

% Incluir el apéndice del API Gateway
\include{codigo_fuente_appendix}

\end{document}
EOF

echo "📄 Ejemplo de inclusión creado: docs/tfg_appendix/ejemplo_inclusion.tex"

echo ""
echo "📊 Apéndice del API Gateway creado:"
echo "  📄 Apéndice: docs/tfg_appendix/codigo_fuente_appendix.tex"
echo "  📋 Ejemplo: docs/tfg_appendix/ejemplo_inclusion.tex"
echo ""
echo "💡 Para usar en tu TFG:"
echo "  1. Copia codigo_fuente_appendix.tex a tu directorio"
echo "  2. Incluye en tu documento: \\include{codigo_fuente_appendix}"
echo "  3. O copia las secciones que necesites"

echo ""
echo "✅ Proceso completado" 