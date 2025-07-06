#!/bin/bash

# Colores para la salida
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # Sin color

# Función para verificar comandos
check_command() {
  if ! command -v $1 &> /dev/null; then
    echo -e "${RED}Error: $1 no está instalado. Por favor, instálalo.${NC}"
    exit 1
  fi
}



# Función para detectar archivos en el directorio actual
detect_files() {
  echo "Detectando archivos en el directorio actual..."
  
  # Detectar metallb-config.yaml
  if [ -f "metallb-config.yaml" ]; then
    echo -e "${GREEN}✓ Encontrado metallb-config.yaml${NC}"
  else
    echo -e "${RED}✗ metallb-config.yaml no encontrado en el directorio actual${NC}"
  fi
  
  # Detectar directorio kustomize
  if [ -d "kustomize" ]; then
    echo -e "${GREEN}✓ Encontrado directorio kustomize${NC}"
  else
    echo -e "${RED}✗ Directorio kustomize no encontrado en el directorio actual${NC}"
  fi
}

# Verificar dependencias
echo "Verificando dependencias..."
check_command minikube
check_command kubectl

# Detectar archivos disponibles
detect_files

# 1. Iniciar Minikube si no está corriendo o está en estado inconsistente
echo "Comprobando estado de Minikube..."
MINIKUBE_STATUS=$(minikube status 2>/dev/null)

# Verificar si minikube está completamente funcional
if echo "$MINIKUBE_STATUS" | grep -q "Running" && \
   echo "$MINIKUBE_STATUS" | grep -q "kubelet: Running" && \
   echo "$MINIKUBE_STATUS" | grep -q "apiserver: Running"; then
  echo -e "${GREEN}Minikube está completamente funcional.${NC}"
else
  echo -e "${YELLOW}Minikube no está completamente funcional.${NC}"
  echo "Estado actual:"
  echo "$MINIKUBE_STATUS"
  
  # Verificar si kubeconfig está desactualizado
  if echo "$MINIKUBE_STATUS" | grep -q "kubeconfig: Misconfigured"; then
    echo "Kubeconfig desactualizado. Actualizando contexto..."
    minikube update-context
    sleep 2
  fi
  
  # Detener minikube si está en estado inconsistente
  if echo "$MINIKUBE_STATUS" | grep -q "host: Running"; then
    echo "Deteniendo minikube para reiniciarlo limpiamente..."
    minikube stop
    sleep 5
  fi
  
  # Reiniciar Docker si es necesario (para mejorar rendimiento)
  echo "Verificando servicio Docker..."
  if command -v docker &> /dev/null; then
    echo "Reiniciando servicio Docker para mejorar rendimiento..."
    sudo systemctl restart docker 2>/dev/null || true
    sleep 3
  fi
  
  echo "Iniciando Minikube..."
  minikube start --driver=docker --force
  
  # Esperar más tiempo para que se estabilice
  echo "Esperando a que minikube se estabilice..."
  sleep 15
  
  # Verificar que se inició correctamente
  NEW_STATUS=$(minikube status 2>/dev/null)
  if echo "$NEW_STATUS" | grep -q "Running" && \
     echo "$NEW_STATUS" | grep -q "kubelet: Running" && \
     echo "$NEW_STATUS" | grep -q "apiserver: Running"; then
    echo -e "${GREEN}Minikube iniciado correctamente.${NC}"
  else
    echo -e "${RED}Error: Minikube no se pudo iniciar correctamente.${NC}"
    echo "Estado final:"
    minikube status
    echo -e "${YELLOW}Intentando reiniciar minikube una vez más...${NC}"
    
    # Último intento: reiniciar completamente
    minikube stop
    sleep 5
    minikube start --driver=docker --force
    sleep 15
    
    FINAL_STATUS=$(minikube status 2>/dev/null)
    if echo "$FINAL_STATUS" | grep -q "kubelet: Running" && \
       echo "$FINAL_STATUS" | grep -q "apiserver: Running"; then
      echo -e "${GREEN}Minikube iniciado correctamente en el segundo intento.${NC}"
    else
      echo -e "${RED}Error: Minikube no se pudo iniciar. Ejecuta cleanup_all.sh y vuelve a intentar.${NC}"
      exit 1
    fi
  fi
fi

# Verificar conectividad con la API antes de continuar
echo "Verificando conectividad con la API de Kubernetes..."
API_RETRY_COUNT=0
API_READY=false

while [ $API_RETRY_COUNT -lt 5 ] && [ "$API_READY" = false ]; do
  if kubectl cluster-info &> /dev/null; then
    echo -e "${GREEN}✓ API de Kubernetes accesible${NC}"
    API_READY=true
  else
    echo -e "${RED}✗ API de Kubernetes NO accesible (intento $((API_RETRY_COUNT + 1))/5)${NC}"
    API_RETRY_COUNT=$((API_RETRY_COUNT + 1))
    
    if [ $API_RETRY_COUNT -lt 5 ]; then
      echo "Esperando 10 segundos antes de reintentar..."
      sleep 10
    fi
  fi
done

if [ "$API_READY" = false ]; then
  echo -e "${RED}Error: No se pudo conectar con la API de Kubernetes después de 5 intentos.${NC}"
  echo "Estado de minikube:"
  minikube status
  exit 1
fi

# 2. Habilitar MetalLB con manejo de errores
echo "Comprobando MetalLB..."
METALLB_ENABLED=false
MAX_RETRIES=3
RETRY_COUNT=0

while [ $RETRY_COUNT -lt $MAX_RETRIES ] && [ "$METALLB_ENABLED" = false ]; do
  if ! minikube addons list | grep metallb | grep -q enabled; then
    echo "Habilitando MetalLB (intento $((RETRY_COUNT + 1))/$MAX_RETRIES)..."
    
    # Intentar habilitar MetalLB
    if minikube addons enable metallb --force; then
      echo -e "${GREEN}MetalLB habilitado exitosamente.${NC}"
      METALLB_ENABLED=true
    else
      echo -e "${RED}Error al habilitar MetalLB.${NC}"
      RETRY_COUNT=$((RETRY_COUNT + 1))
      
      if [ $RETRY_COUNT -lt $MAX_RETRIES ]; then
        echo "Esperando 10 segundos antes de reintentar..."
        sleep 10
        
        # Verificar si minikube está funcionando
        if ! minikube status | grep -q "Running"; then
          echo "Reiniciando minikube..."
          minikube stop
          minikube start --driver=docker
        fi
      fi
    fi
  else
    echo -e "${GREEN}MetalLB ya está habilitado.${NC}"
    METALLB_ENABLED=true
  fi
done

if [ "$METALLB_ENABLED" = false ]; then
  echo -e "${RED}Error: No se pudo habilitar MetalLB después de $MAX_RETRIES intentos.${NC}"
  echo "Intentando instalación manual de MetalLB..."
  
  # Instalación manual de MetalLB
  echo "Instalando MetalLB manualmente..."
  kubectl apply -f https://raw.githubusercontent.com/metallb/metallb/v0.14.8/config/manifests/metallb-native.yaml --validate=false
  
  # Esperar a que los pods estén listos
  echo "Esperando a que los pods de MetalLB estén listos..."
  kubectl wait --namespace metallb-system \
    --for=condition=ready pod \
    --selector=app=metallb \
    --timeout=120s
fi

# 3. Verificar e instalar CRDs de MetalLB
echo "Comprobando CRDs de MetalLB..."
CRD_RETRY_COUNT=0
CRD_INSTALLED=false

while [ $CRD_RETRY_COUNT -lt $MAX_RETRIES ] && [ "$CRD_INSTALLED" = false ]; do
  if ! kubectl get crds | grep -q ipaddresspools.metallb.io; then
    echo "Instalando manifiesto de MetalLB (intento $((CRD_RETRY_COUNT + 1))/$MAX_RETRIES)..."
    
    # Intentar instalar con validación deshabilitada
    if kubectl apply -f https://raw.githubusercontent.com/metallb/metallb/v0.14.8/config/manifests/metallb-native.yaml --validate=false; then
      echo -e "${GREEN}Manifiesto de MetalLB instalado exitosamente.${NC}"
      CRD_INSTALLED=true
    else
      echo -e "${RED}Error al instalar manifiesto de MetalLB.${NC}"
      CRD_RETRY_COUNT=$((CRD_RETRY_COUNT + 1))
      
      if [ $CRD_RETRY_COUNT -lt $MAX_RETRIES ]; then
        echo "Esperando 15 segundos antes de reintentar..."
        sleep 15
        
        # Verificar conectividad con la API
        if ! kubectl cluster-info &> /dev/null; then
          echo "Problemas de conectividad con la API. Reiniciando minikube..."
          minikube stop
          minikube start --driver=docker
        fi
      fi
    fi
  else
    echo -e "${GREEN}CRDs de MetalLB ya están instalados.${NC}"
    CRD_INSTALLED=true
  fi
done

if [ "$CRD_INSTALLED" = false ]; then
  echo -e "${RED}Error: No se pudieron instalar los CRDs de MetalLB.${NC}"
  echo "Continuando sin MetalLB..."
fi

# 4. Aplicar metallb-config.yaml si el IPAddressPool no existe
echo "Comprobando IPAddressPool..."
if ! kubectl get ipaddresspools -n metallb-system custom-ip-pool &> /dev/null; then
  echo "Aplicando metallb-config.yaml..."
  
  # Verificar que el archivo existe en el directorio actual
  if [ -f "metallb-config.yaml" ]; then
    echo "Usando metallb-config.yaml del directorio actual"
    kubectl apply -f metallb-config.yaml --validate=false
  else
    echo -e "${RED}Error: metallb-config.yaml no encontrado en el directorio actual.${NC}"
    echo "Creando configuración básica de MetalLB..."
    
    # Crear configuración básica
    cat > metallb-config.yaml << EOF
apiVersion: metallb.io/v1beta1
kind: IPAddressPool
metadata:
  name: custom-ip-pool
  namespace: metallb-system
spec:
  addresses:
  - 172.17.255.1-172.17.255.255
---
apiVersion: metallb.io/v1beta1
kind: L2Advertisement
metadata:
  name: custom-l2-advertisement
  namespace: metallb-system
spec:
  ipAddressPools:
  - custom-ip-pool
EOF
    
    kubectl apply -f metallb-config.yaml --validate=false
  fi
else
  echo -e "${GREEN}IPAddressPool ya está configurado.${NC}"
fi

# Verificación final del estado de MetalLB
echo -e "\n${GREEN}Verificando estado final de MetalLB...${NC}"
if kubectl get pods -n metallb-system &> /dev/null; then
  echo "Pods de MetalLB:"
  kubectl get pods -n metallb-system
  echo -e "\nIPAddressPools:"
  kubectl get ipaddresspools -n metallb-system || echo "No se encontraron IPAddressPools"
  echo -e "\nL2Advertisements:"
  kubectl get l2advertisements -n metallb-system || echo "No se encontraron L2Advertisements"
else
  echo -e "${RED}MetalLB no está instalado o no funciona correctamente.${NC}"
fi

# 5. Instalar Metrics Server si no está presente
echo "Comprobando Metrics Server..."
if ! kubectl get pods -n kube-system | grep -q metrics-server; then
  echo "Instalando Metrics Server..."
  kubectl apply -f https://github.com/kubernetes-sigs/metrics-server/releases/latest/download/components.yaml
else
  echo -e "${GREEN}Metrics Server ya está instalado.${NC}"
fi

# 6. Aplicar recursos de Kubernetes con Kustomize
echo "Aplicando recursos de Kubernetes con Kustomize..."
if [ -d "kustomize" ]; then
  kubectl apply -k kustomize/
else
  echo -e "${RED}Error: Directorio kustomize no encontrado en el directorio actual.${NC}"
  echo "Buscando en directorio padre..."
  if [ -d "../kustomize" ]; then
    echo "Usando kustomize del directorio padre"
    kubectl apply -k ../kustomize/
  else
    echo -e "${RED}Error: Directorio kustomize no encontrado.${NC}"
    exit 1
  fi
fi

# 7. Verificar el estado
echo -e "\n${GREEN}Verificando estado del despliegue...${NC}"
echo "Servicios:"
kubectl get services
echo -e "\nPods:"
kubectl get pods
echo -e "\nHPA:"
kubectl get hpa

# 8. Verificar logs del pod
sleep 5
echo -e "\nMostrando logs del pod..."

POD_NAME=$(kubectl get pods -l app=servidor-central -o jsonpath="{.items[0].metadata.name}")
if [ -n "$POD_NAME" ]; then
  kubectl logs -f $POD_NAME
else
  echo -e "${RED}No se encontró el pod del servidor-central.${NC}"
fi