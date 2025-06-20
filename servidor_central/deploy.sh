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

# Verificar dependencias
echo "Verificando dependencias..."
check_command minikube
check_command kubectl

# 1. Iniciar Minikube si no está corriendo
echo "Comprobando estado de Minikube..."
if ! minikube status | grep -q "Running"; then
  echo "Iniciando Minikube..."
  minikube start --driver=docker
else
  echo -e "${GREEN}Minikube ya está corriendo.${NC}"
fi

# 2. Habilitar MetalLB si no está habilitado
echo "Comprobando MetalLB..."
if ! minikube addons list | grep metallb | grep -q enabled; then
  echo "Habilitando MetalLB..."
  minikube addons enable metallb
else
  echo -e "${GREEN}MetalLB ya está habilitado.${NC}"
fi

# 3. Instalar manifiesto de MetalLB si los CRDs no están presentes
echo "Comprobando CRDs de MetalLB..."
if ! kubectl get crds | grep -q ipaddresspools.metallb.io; then
  echo "Instalando manifiesto de MetalLB..."
  kubectl apply -f https://raw.githubusercontent.com/metallb/metallb/v0.14.8/config/manifests/metallb-native.yaml
else
  echo -e "${GREEN}CRDs de MetalLB ya están instalados.${NC}"
fi

# 4. Aplicar metallb-config.yaml si el IPAddressPool no existe
echo "Comprobando IPAddressPool..."
if ! kubectl get ipaddresspools -n metallb-system custom-ip-pool &> /dev/null; then
  echo "Aplicando metallb-config.yaml..."
  kubectl apply -f metallb-config.yaml
else
  echo -e "${GREEN}IPAddressPool ya está configurado.${NC}"
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
  echo -e "${RED}Error: Directorio kustomize no encontrado.${NC}"
  exit 1
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