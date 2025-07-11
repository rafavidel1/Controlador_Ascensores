% !TEX root =../LibroTipoETSI.tex
% Definir entorno observacion
\newenvironment{observacion}{\begin{quote}\textbf{Observación:} }{\end{quote}}

% Iniciar apéndices
\appendix

% APENDICE A
\chapter{Guía de Instalación y Ejecución}\label{ApA}
Este apéndice describe cómo instalar, compilar y ejecutar el sistema de control de ascensores usando los scripts automatizados.

\section{Requisitos del Sistema}\label{sec:requisitos}

\lettrine[lraise=-0.1, lines=2, loversize=0.2]{E}{l} sistema requiere únicamente:

\begin{itemize}
    \item \textbf{Sistema Operativo}: Ubuntu 22.04+ / Debian 11+ / CentOS 8+
    \item \textbf{Permisos}: sudo para instalación de dependencias
    \item \textbf{Conexión a Internet}: Para descargar dependencias
\end{itemize}

\textbf{Importante}: No es necesario instalar manualmente cmake, gcc, libcoap, Docker, Kubernetes ni ninguna otra dependencia. Los scripts instalan todo automáticamente.

\section{Scripts Principales}\label{sec:scripts}

El sistema utiliza 4 scripts principales que deben ejecutarse en orden:

\begin{enumerate}
    \item \textbf{build\_api\_gateway.sh}: Instala dependencias y compila el API Gateway
    \item \textbf{build\_servidor\_central.sh}: Instala dependencias y compila el servidor central  
    \item \textbf{deploy.sh}: Despliega el servidor central en Kubernetes
    \item \textbf{run\_100\_api\_gateways.sh}: Ejecuta múltiples instancias del API Gateway para pruebas
\end{enumerate}

\section{Script 1: build\_api\_gateway.sh}\label{sec:build-api}

\subsection{Propósito}
Este script instala todas las dependencias necesarias y compila el API Gateway automáticamente.

\subsection{Qué hace el script}
\begin{enumerate}
    \item Instala herramientas de compilación: build-essential, cmake, gcc, make, pkg-config, git
    \item Instala librerías adicionales: libtool, autoconf, automake, wget, ca-certificates
    \item Instala OpenSSL: libssl-dev
    \item Instala cJSON: libcjson-dev
    \item Instala json-c: libjson-c-dev
    \item Descarga, compila e instala libcoap desde el repositorio oficial
    \item Configura variables de entorno PKG\_CONFIG\_PATH y LD\_LIBRARY\_PATH
    \item Ejecuta cmake para configurar el proyecto
    \item Compila el API Gateway usando make
    \item Copia archivos de simulación necesarios
\end{enumerate}

\subsection{Cómo ejecutarlo}
\begin{lstlisting}[language=bash,caption={Compilación del API Gateway},label=prg:build-api]
# Navegar al directorio del API Gateway
cd api_gateway/

# Ejecutar el script de compilación
./build_api_gateway.sh

# Una vez compilado, ejecutar el API Gateway
./api_gateway
\end{lstlisting}

\subsection{Resultado esperado}
\begin{itemize}
    \item Se instalan todas las dependencias automáticamente
    \item Se compila el ejecutable \texttt{api\_gateway}
    \item Se puede ejecutar el API Gateway con \texttt{./api\_gateway}
\end{itemize}

\section{Script 2: build\_servidor\_central.sh}\label{sec:build-servidor}

\subsection{Propósito}
Este script instala todas las dependencias necesarias y compila el servidor central.

\subsection{Qué hace el script}
\begin{enumerate}
    \item Instala las mismas dependencias que el API Gateway
    \item Instala libcurl adicional: libcurl4-openssl-dev
    \item Descarga, compila e instala libcoap si no está instalado
    \item Configura variables de entorno
    \item Ejecuta cmake para configurar el proyecto
    \item Compila el servidor central usando make
\end{enumerate}

\subsection{Cómo ejecutarlo}
\begin{lstlisting}[language=bash,caption={Compilación del servidor central},label=prg:build-servidor]
# Navegar al directorio del servidor central
cd servidor_central/

# Ejecutar el script de compilación
./build_servidor_central.sh

# Una vez compilado, ejecutar el servidor localmente
./build/servidor_central
\end{lstlisting}

\subsection{Resultado esperado}
\begin{itemize}
    \item Se instalan todas las dependencias automáticamente
    \item Se compila el ejecutable \texttt{servidor\_central}
    \item Se puede ejecutar el servidor con \texttt{./build/servidor\_central}
\end{itemize}

\section{Script 3: deploy.sh}\label{sec:deploy}

\subsection{Propósito}
Este script despliega el servidor central en Kubernetes usando minikube.

\subsection{Qué hace el script}
\begin{enumerate}
    \item Verifica que minikube y kubectl estén instalados
    \item Inicia minikube si no está funcionando
    \item Configura el entorno Docker de minikube
    \item Construye la imagen Docker del servidor central
    \item Instala MetalLB para LoadBalancer
    \item Configura pool de IPs: 192.168.49.2-192.168.49.10
    \item Instala Metrics Server para métricas
    \item Aplica los manifiestos de Kubernetes usando kustomize
    \item Verifica que el despliegue esté funcionando
\end{enumerate}

\subsection{Cómo ejecutarlo}
\begin{lstlisting}[language=bash,caption={Despliegue en Kubernetes},label=prg:deploy]
# Navegar al directorio del servidor central
cd servidor_central/

# Ejecutar el script de despliegue
./deploy.sh

# El servidor estará disponible en:
# http://192.168.49.2:5684
\end{lstlisting}

\subsection{Resultado esperado}
\begin{itemize}
    \item Se despliega el servidor central en Kubernetes
    \item Se configura MetalLB para LoadBalancer
    \item El servidor está disponible en http://192.168.49.2:5684
    \item Se puede verificar con \texttt{kubectl get pods}
\end{itemize}

\section{Script 4: run\_100\_api\_gateways.sh}\label{sec:run-100}

\subsection{Propósito}
Este script ejecuta múltiples instancias del API Gateway simultáneamente para pruebas de carga.

\subsection{Qué hace el script}
\begin{enumerate}
    \item Verifica que el servidor central esté accesible
    \item Crea el directorio de logs masivos
    \item Inicia múltiples instancias del API Gateway
    \item Asigna puertos únicos a cada instancia (6000-6099)
    \item Configura variables de entorno únicas para cada instancia
    \item Ejecuta cada instancia en background
    \item Monitorea las instancias durante el tiempo especificado
    \item Termina todas las instancias de forma controlada
    \item Genera un reporte final con estadísticas
\end{enumerate}

\subsection{Cómo ejecutarlo}
\begin{lstlisting}[language=bash,caption={Ejecución masiva de API Gateways},label=prg:run-100]
# Navegar al directorio del API Gateway
cd api_gateway/

# Ejecutar 100 instancias por 30 segundos
./run_100_api_gateways.sh

# Ejecutar con configuración personalizada
./run_100_api_gateways.sh -n 50 -b 7000 -t 60
# 50 instancias, puerto base 7000, por 60 segundos
\end{lstlisting}

\subsection{Resultado esperado}
\begin{itemize}
    \item Se ejecutan múltiples instancias del API Gateway
    \item Cada instancia usa un puerto único
    \item Se generan logs separados para cada instancia
    \item Se crea un reporte final con estadísticas
\end{itemize}

\section{Flujo de Ejecución Completo}\label{sec:flujo}

Para ejecutar el sistema completo, seguir este orden:

\begin{enumerate}
    \item \textbf{Compilar API Gateway}:
        \begin{lstlisting}[language=bash]
cd api_gateway/
./build_api_gateway.sh
        \end{lstlisting}
    
    \item \textbf{Compilar servidor central}:
        \begin{lstlisting}[language=bash]
cd ../servidor_central/
./build_servidor_central.sh
        \end{lstlisting}
    
    \item \textbf{Desplegar servidor en Kubernetes}:
        \begin{lstlisting}[language=bash]
./deploy.sh
        \end{lstlisting}
    
    \item \textbf{Ejecutar API Gateway individual}:
        \begin{lstlisting}[language=bash]
cd ../api_gateway/
./api_gateway
        \end{lstlisting}
    
    \item \textbf{O ejecutar múltiples instancias}:
        \begin{lstlisting}[language=bash]
./run_100_api_gateways.sh
        \end{lstlisting}
\end{enumerate}

\section{Ejecución de Pruebas}\label{sec:pruebas}

El sistema incluye un script adicional para ejecutar todas las pruebas:

\begin{lstlisting}[language=bash,caption={Ejecución de todas las pruebas},label=prg:tests]
# Navegar al directorio de pruebas
cd tests/

# Ejecutar todas las pruebas
./run_all_tests.sh
\end{lstlisting}

\subsection{Qué hace run\_all\_tests.sh}
\begin{enumerate}
    \item Verifica dependencias: CMake, GCC, libcoap, libcjson, CUnit
    \item Configura CMake con flags de testing
    \item Compila todas las pruebas
    \item Ejecuta 34 pruebas en 5 módulos diferentes
    \item Ejecuta también CTest para verificación adicional
    \item Genera reportes detallados de resultados
\end{enumerate}

\section{Verificación del Sistema}\label{sec:verificacion}

Para verificar que todo funciona correctamente:

\begin{enumerate}
    \item \textbf{Verificar que el API Gateway compila}:
        \begin{lstlisting}[language=bash]
cd api_gateway/
ls -la api_gateway  # Debe existir el ejecutable
        \end{lstlisting}
    
    \item \textbf{Verificar que el servidor central compila}:
        \begin{lstlisting}[language=bash]
cd servidor_central/
ls -la build/servidor_central  # Debe existir el ejecutable
        \end{lstlisting}
    
    \item \textbf{Verificar despliegue en Kubernetes}:
        \begin{lstlisting}[language=bash]
kubectl get pods          # Debe mostrar el pod del servidor
kubectl get services      # Debe mostrar el servicio LoadBalancer
        \end{lstlisting}
    
    \item \textbf{Verificar conectividad}:
        \begin{lstlisting}[language=bash]
curl -k https://192.168.49.2:5684/status  # Debe responder el servidor
        \end{lstlisting}
\end{enumerate}

\section{Limpieza del Sistema}\label{sec:limpieza}

Para limpiar el sistema después de las pruebas:

\begin{lstlisting}[language=bash,caption={Limpieza del sistema},label=prg:cleanup]
# Limpiar Kubernetes
cd servidor_central/
./cleanup_all.sh

# Limpiar logs masivos
cd api_gateway/
./run_100_api_gateways.sh -c

# Limpiar builds de pruebas
cd tests/
./clean_temp_build.sh
\end{lstlisting}

\section{Archivos Importantes}\label{sec:archivos}

\subsection{Archivos de Configuración}
\begin{itemize}
    \item \textbf{gateway.env}: Configuración del API Gateway
    \item \textbf{simulation\_data.json}: Datos de simulación de edificios
    \item \textbf{psk\_keys.txt}: Claves PSK para DTLS
    \item \textbf{metallb-config.yaml}: Configuración de MetalLB
    \item \textbf{kustomize/}: Manifiestos de Kubernetes
\end{itemize}

\subsection{Archivos de Logs}
\begin{itemize}
    \item \textbf{logs/YYYY-MM-DD/}: Logs del API Gateway por fecha
    \item \textbf{mass\_execution\_logs/}: Logs de ejecución masiva
    \item \textbf{tests/temp-build-tests/}: Reportes de pruebas
\end{itemize}

\subsection{Ejecutables Generados}
\begin{itemize}
    \item \textbf{api\_gateway/api\_gateway}: Ejecutable del API Gateway
    \item \textbf{servidor\_central/build/servidor\_central}: Ejecutable del servidor central
    \item \textbf{tests/temp-build-tests/tests/}: Ejecutables de pruebas
\end{itemize}

\section{Solución de Problemas}\label{sec:problemas}

\subsection{Problemas Comunes}

\begin{itemize}
    \item \textbf{Error "Permission denied"}: Usar \texttt{chmod +x script.sh} antes de ejecutar
    \item \textbf{Error "libcoap not found"}: Ejecutar \texttt{./build\_api\_gateway.sh} para instalar
    \item \textbf{Error "minikube not found"}: Instalar minikube manualmente
    \item \textbf{Error "Port already in use"}: Cambiar puerto con \texttt{./api\_gateway 6000}
    \item \textbf{Error en compilación}: Verificar que se tienen permisos de sudo
\end{itemize}

\subsection{Comandos de Diagnóstico}

\begin{lstlisting}[language=bash,caption={Comandos de diagnóstico},label=prg:diagnostico]
# Verificar dependencias
cmake --version
gcc --version
pkg-config --exists libcoap-3-openssl && echo "libcoap OK"

# Verificar Kubernetes
kubectl get nodes
kubectl get pods
minikube status

# Verificar logs
tail -f api_gateway/logs/$(date +%Y-%m-%d)/*.md
kubectl logs -f deployment/servidor-central
\end{lstlisting}

Este apéndice proporciona toda la información necesaria para instalar, compilar y ejecutar el sistema de control de ascensores usando los scripts automatizados.








