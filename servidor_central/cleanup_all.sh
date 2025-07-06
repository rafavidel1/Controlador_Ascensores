#!/bin/bash

# Colores para la salida
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # Sin color

echo -e "${BLUE}🧹 Script de Limpieza Completa - Kubernetes & MetalLB${NC}"
echo "========================================================"

# Función para mostrar progreso
show_progress() {
  echo -e "${BLUE}🔄 $1${NC}"
}

# Función para mostrar éxito
show_success() {
  echo -e "${GREEN}✓ $1${NC}"
}

# Función para mostrar error
show_error() {
  echo -e "${RED}✗ $1${NC}"
}

# Función para mostrar advertencia
show_warning() {
  echo -e "${YELLOW}⚠️  $1${NC}"
}

echo -e "\n${YELLOW}¿Estás seguro de que quieres limpiar todo? (y/N)${NC}"
read -r response
if [[ ! "$response" =~ ^[Yy]$ ]]; then
  echo "Limpieza cancelada."
  exit 0
fi

echo -e "\n${BLUE}Iniciando limpieza completa...${NC}"

# 1. Limpiar recursos de Kubernetes
show_progress "1. Limpiando recursos de Kubernetes..."

# Eliminar recursos de kustomize si existe
if [ -d "kustomize" ]; then
  show_progress "   Eliminando recursos de kustomize..."
  kubectl delete -k kustomize/ --ignore-not-found=true 2>/dev/null || true
  show_success "   Recursos de kustomize eliminados"
fi

# Eliminar servicios LoadBalancer
show_progress "   Eliminando servicios LoadBalancer..."
kubectl get services --all-namespaces -o jsonpath='{range .items[?(@.spec.type=="LoadBalancer")]}{.metadata.namespace}/{.metadata.name}{"\n"}{end}' 2>/dev/null | while read service; do
  if [ -n "$service" ]; then
    namespace=$(echo "$service" | cut -d'/' -f1)
    name=$(echo "$service" | cut -d'/' -f2)
    kubectl delete service "$name" -n "$namespace" --ignore-not-found=true 2>/dev/null || true
    show_success "     Servicio $name eliminado"
  fi
done

# Eliminar todos los pods del servidor central
show_progress "   Eliminando pods del servidor central..."
kubectl delete pods -l app=servidor-central --all-namespaces --ignore-not-found=true 2>/dev/null || true
kubectl delete deployments -l app=servidor-central --all-namespaces --ignore-not-found=true 2>/dev/null || true

# 2. Limpiar MetalLB
show_progress "2. Limpiando MetalLB..."

# Eliminar configuración de MetalLB
show_progress "   Eliminando configuración de MetalLB..."
kubectl delete ipaddresspools --all -n metallb-system --ignore-not-found=true 2>/dev/null || true
kubectl delete l2advertisements --all -n metallb-system --ignore-not-found=true 2>/dev/null || true
kubectl delete bgppeers --all -n metallb-system --ignore-not-found=true 2>/dev/null || true
kubectl delete bgpadvertisements --all -n metallb-system --ignore-not-found=true 2>/dev/null || true

# Eliminar manifiesto de MetalLB
show_progress "   Eliminando manifiesto de MetalLB..."
kubectl delete -f https://raw.githubusercontent.com/metallb/metallb/v0.14.8/config/manifests/metallb-native.yaml --ignore-not-found=true 2>/dev/null || true

# Eliminar namespace de MetalLB
show_progress "   Eliminando namespace metallb-system..."
kubectl delete namespace metallb-system --ignore-not-found=true 2>/dev/null || true

# Deshabilitar addon de MetalLB
show_progress "   Deshabilitando addon de MetalLB..."
minikube addons disable metallb 2>/dev/null || true

# 3. Limpiar Metrics Server
show_progress "3. Limpiando Metrics Server..."
kubectl delete -f https://github.com/kubernetes-sigs/metrics-server/releases/latest/download/components.yaml --ignore-not-found=true 2>/dev/null || true

# 4. Limpiar minikube
show_progress "4. Limpiando minikube..."

# Detener minikube
show_progress "   Deteniendo minikube..."
minikube stop 2>/dev/null || true
show_success "   Minikube detenido"

# Eliminar minikube completamente
show_progress "   Eliminando minikube..."
minikube delete --all --purge 2>/dev/null || true
show_success "   Minikube eliminado completamente"

# 5. Limpiar Docker (opcional)
echo -e "\n${YELLOW}¿Quieres también limpiar contenedores Docker no utilizados? (y/N)${NC}"
read -r docker_clean
if [[ "$docker_clean" =~ ^[Yy]$ ]]; then
  show_progress "5. Limpiando Docker..."
  
  # Eliminar contenedores no utilizados
  show_progress "   Eliminando contenedores no utilizados..."
  docker container prune -f 2>/dev/null || true
  
  # Eliminar imágenes no utilizadas
  show_progress "   Eliminando imágenes no utilizadas..."
  docker image prune -f 2>/dev/null || true
  
  # Eliminar volúmenes no utilizados
  show_progress "   Eliminando volúmenes no utilizados..."
  docker volume prune -f 2>/dev/null || true
  
  # Eliminar redes no utilizadas
  show_progress "   Eliminando redes no utilizadas..."
  docker network prune -f 2>/dev/null || true
  
  show_success "   Docker limpiado"
fi

# 6. Limpiar archivos temporales
show_progress "6. Limpiando archivos temporales..."

# Eliminar archivo de configuración de MetalLB si fue creado automáticamente
if [ -f "metallb-config.yaml" ]; then
  show_progress "   Eliminando metallb-config.yaml..."
  rm -f metallb-config.yaml
  show_success "   metallb-config.yaml eliminado"
fi

# 7. Verificar limpieza
show_progress "7. Verificando limpieza..."

# Verificar que minikube no está corriendo
if ! minikube status &> /dev/null; then
  show_success "   Minikube no está corriendo"
else
  show_warning "   Minikube aún está presente"
fi

# Verificar que no hay pods corriendo
if ! kubectl get pods --all-namespaces &> /dev/null; then
  show_success "   No hay pods corriendo"
else
  show_warning "   Aún hay pods corriendo"
fi

# Resumen final
echo -e "\n${GREEN}🎉 Limpieza completada${NC}"
echo "================================"
echo -e "${BLUE}Se han eliminado:${NC}"
echo "  • Recursos de Kubernetes"
echo "  • MetalLB y su configuración"
echo "  • Metrics Server"
echo "  • Minikube completamente"
if [[ "$docker_clean" =~ ^[Yy]$ ]]; then
  echo "  • Contenedores Docker no utilizados"
fi
echo "  • Archivos temporales"

echo -e "\n${YELLOW}Para volver a desplegar, ejecuta:${NC}"
echo "  ./deploy.sh"

echo -e "\n${BLUE}¡Todo limpio! 🧹${NC}" 