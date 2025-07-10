#!/bin/bash

# Colores para la salida
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # Sin color

# Función para reinstalar MetalLB completamente cuando esté en mal estado
reinstall_metallb() {
  echo -e "${YELLOW}🔄 Reinstalando MetalLB completamente...${NC}"
  
  # 1. PRIMERO eliminar todos los CRDs de MetalLB (esto libera el namespace)
  echo "Eliminando CRDs de MetalLB PRIMERO..."
  kubectl delete crd ipaddresspools.metallb.io --force --grace-period=0 2>/dev/null || true
  kubectl delete crd l2advertisements.metallb.io --force --grace-period=0 2>/dev/null || true
  kubectl delete crd bgpadvertisements.metallb.io --force --grace-period=0 2>/dev/null || true
  kubectl delete crd bgppeers.metallb.io --force --grace-period=0 2>/dev/null || true
  kubectl delete crd addresspools.metallb.io --force --grace-period=0 2>/dev/null || true
  kubectl delete crd bfdprofiles.metallb.io --force --grace-period=0 2>/dev/null || true
  kubectl delete crd communities.metallb.io --force --grace-period=0 2>/dev/null || true
  
  # 2. Eliminar todos los recursos de MetalLB en el namespace
  echo "Eliminando recursos de MetalLB..."
  kubectl delete all --all -n metallb-system --force --grace-period=0 2>/dev/null || true
  kubectl delete configmap --all -n metallb-system --force --grace-period=0 2>/dev/null || true
  kubectl delete secret --all -n metallb-system --force --grace-period=0 2>/dev/null || true
  kubectl delete serviceaccount --all -n metallb-system --force --grace-period=0 2>/dev/null || true
  
  # 3. Eliminar roles y bindings de MetalLB
  echo "Eliminando roles y bindings de MetalLB..."
  kubectl delete clusterrole metallb-system:controller 2>/dev/null || true
  kubectl delete clusterrole metallb-system:speaker 2>/dev/null || true
  kubectl delete clusterrolebinding metallb-system:controller 2>/dev/null || true
  kubectl delete clusterrolebinding metallb-system:speaker 2>/dev/null || true
  kubectl delete role -n metallb-system --all --force --grace-period=0 2>/dev/null || true
  kubectl delete rolebinding -n metallb-system --all --force --grace-period=0 2>/dev/null || true
  
  # 3b. Eliminar webhooks de MetalLB que pueden bloquear la eliminación
  echo "Eliminando webhooks de MetalLB..."
  kubectl delete validatingwebhookconfiguration metallb-webhook-configuration 2>/dev/null || true
  kubectl delete mutatingwebhookconfiguration metallb-webhook-configuration 2>/dev/null || true
  
  # 4. Ahora eliminar el namespace y forzar si está atascado
  echo "Eliminando namespace metallb-system..."
  kubectl delete namespace metallb-system --force --grace-period=0 2>/dev/null || true
  
  # 5. Si sigue atascado en "Terminating", forzar eliminación de finalizers
  sleep 3
  if kubectl get namespace metallb-system 2>/dev/null | grep -q "Terminating"; then
    echo "Namespace atascado en 'Terminating'. Eliminando finalizers..."
    # Método 1: Patch directo para eliminar finalizers
    kubectl patch namespace metallb-system -p '{"metadata":{"finalizers":null}}' --type=merge 2>/dev/null || true
    # Método 2: Patch usando spec.finalizers vacío
    kubectl patch namespace metallb-system -p '{"spec":{"finalizers":[]}}' --type=merge 2>/dev/null || true
    # Método 3: Usar kubectl replace con JSON directo (sin jq)
    echo '{"apiVersion":"v1","kind":"Namespace","metadata":{"name":"metallb-system"},"spec":{"finalizers":[]}}' | kubectl replace --raw /api/v1/namespaces/metallb-system/finalize -f - 2>/dev/null || true
  fi
  
  # 6. Esperar a que se elimine completamente (con timeout)
  echo "Esperando a que se elimine completamente el namespace..."
  TIMEOUT=30
  COUNT=0
  while kubectl get namespace metallb-system 2>/dev/null && [ $COUNT -lt $TIMEOUT ]; do
    echo "Esperando eliminación del namespace... ($COUNT/$TIMEOUT)"
    sleep 2
    COUNT=$((COUNT + 1))
  done
  
  # 7. Si aún existe después del timeout, mostrar advertencia pero continuar
  if kubectl get namespace metallb-system 2>/dev/null; then
    echo -e "${YELLOW}⚠ Namespace aún existe después de $TIMEOUT intentos.${NC}"
    echo -e "${YELLOW}  Estado actual del namespace:${NC}"
    kubectl get namespace metallb-system -o wide 2>/dev/null || true
    echo -e "${YELLOW}  Continuando con reinstalación de MetalLB...${NC}"
    echo -e "${BLUE}  💡 El nuevo MetalLB creará un namespace fresco${NC}"
  else
    echo -e "${GREEN}✓ Namespace metallb-system eliminado exitosamente${NC}"
  fi
  
  # 8. Reinstalar MetalLB desde cero
  echo "Reinstalando MetalLB..."
  kubectl apply -f https://raw.githubusercontent.com/metallb/metallb/v0.13.7/config/manifests/metallb-native.yaml
  
  # 9. Esperar a que los pods estén listos
  echo "Esperando a que MetalLB esté listo..."
  kubectl wait --namespace metallb-system \
    --for=condition=ready pod \
    --selector=app=metallb \
    --timeout=300s
  
  # 10. Aplicar configuración
  echo "Aplicando configuración de MetalLB..."
  sleep 10
  kubectl apply -f metallb-config.yaml
  
  if [ $? -eq 0 ]; then
    # 11. Verificación final del estado
    echo "Verificando estado final de MetalLB..."
    sleep 5
    
    # Mostrar estado de pods
    echo -e "${BLUE}Pods de MetalLB:${NC}"
    kubectl get pods -n metallb-system -o wide 2>/dev/null || echo "No se pudieron obtener pods"
    
    # Mostrar configuración aplicada
    echo -e "${BLUE}IPAddressPools configurados:${NC}"
    kubectl get ipaddresspools -n metallb-system 2>/dev/null || echo "No se encontraron IPAddressPools"
    
    echo -e "${GREEN}✓ MetalLB reinstalado exitosamente${NC}"
    return 0
  else
    echo -e "${RED}✗ Error al aplicar configuración de MetalLB${NC}"
    return 1
  fi
}

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

# Función para verificar imagen Docker en contexto de Minikube
check_docker_image() {
  echo "Verificando imagen Docker en contexto de Minikube..."
  
  # Verificar que minikube esté funcionando
  if ! minikube status | grep -q "Running"; then
    echo -e "${YELLOW}Minikube no está funcionando. Iniciando minikube...${NC}"
    minikube start --driver=docker
    sleep 10
  fi
  
  # Configurar entorno de Docker de Minikube automáticamente
  echo "Configurando entorno de Docker de Minikube..."
  if eval $(minikube docker-env 2>/dev/null); then
    echo -e "${GREEN}✓ Entorno de Docker de Minikube configurado${NC}"
    
    # Verificar si la imagen existe y mostrar información
    if docker images | grep -q "servidor-central.*latest"; then
      IMAGE_ID=$(docker images servidor-central:latest --format "table {{.ID}}" | tail -n 1)
      IMAGE_SIZE=$(docker images servidor-central:latest --format "table {{.Size}}" | tail -n 1)
      IMAGE_CREATED=$(docker images servidor-central:latest --format "table {{.CreatedSince}}" | tail -n 1)
      
      echo -e "${GREEN}✓ Imagen servidor-central:latest encontrada en contexto de Minikube${NC}"
      echo -e "${GREEN}  ID: $IMAGE_ID${NC}"
      echo -e "${GREEN}  Tamaño: $IMAGE_SIZE${NC}"
      echo -e "${GREEN}  Creada: $IMAGE_CREATED${NC}"
      
      # Por simplicidad, por ahora solo verificamos si la imagen existe
      # En futuras versiones se puede implementar detección más sofisticada de cambios
      
      return 0
    else
      echo -e "${YELLOW}⚠ Imagen servidor-central:latest NO encontrada en contexto de Minikube${NC}"
      return 1
    fi
  else
    echo -e "${YELLOW}No se pudo configurar el entorno de Docker de Minikube.${NC}"
    return 1
  fi
}

# Función para construir imagen Docker
build_docker_image() {
  echo "Construyendo imagen Docker..."
  
  # Verificar que minikube esté funcionando
  if ! minikube status | grep -q "Running"; then
    echo -e "${YELLOW}Minikube no está funcionando. Iniciando minikube...${NC}"
    minikube start --driver=docker
    sleep 10
  fi
  
  # Configurar entorno de Docker de Minikube automáticamente
  echo "Configurando entorno de Docker de Minikube..."
  if eval $(minikube docker-env 2>/dev/null); then
    echo -e "${GREEN}✓ Entorno de Docker de Minikube configurado${NC}"
    
    # Eliminar imagen anterior si existe
    echo "Verificando y eliminando imagen anterior..."
    if docker images | grep -q "servidor-central.*latest"; then
      echo -e "${YELLOW}Eliminando imagen anterior servidor-central:latest...${NC}"
      docker rmi servidor-central:latest --force 2>/dev/null || true
      echo -e "${GREEN}✓ Imagen anterior eliminada${NC}"
    fi
    
    # Limpieza mínima y segura: solo eliminar imágenes builder específicas
    echo "Limpieza segura: conservando todas las imágenes del sistema..."
    
    # Solo limpiar imágenes que tengan etiquetas específicas de build stages
    # Esto es completamente seguro ya que no afecta imágenes de sistema
    docker builder prune --force 2>/dev/null || true
    
    echo "✓ Limpieza completada sin afectar imágenes del sistema (MetalLB, Minikube, etc.)"
    
    # Construir la imagen
    echo "Construyendo nueva imagen servidor-central:latest..."
    if docker build -t servidor-central:latest --no-cache .; then
      echo -e "${GREEN}✓ Imagen servidor-central:latest construida exitosamente${NC}"
      
      # Verificar que la imagen se creó correctamente
      if docker images | grep -q "servidor-central.*latest"; then
        IMAGE_SIZE=$(docker images servidor-central:latest --format "table {{.Size}}" | tail -n 1)
        echo -e "${GREEN}✓ Nueva imagen creada correctamente (Tamaño: $IMAGE_SIZE)${NC}"
        return 0
      else
        echo -e "${RED}✗ Error: La imagen no aparece en el registro de Docker${NC}"
        return 1
      fi
    else
      echo -e "${RED}✗ Error al construir la imagen Docker${NC}"
      return 1
    fi
  else
    echo -e "${RED}Error: No se pudo configurar el entorno de Docker de Minikube${NC}"
    return 1
  fi
}

# Verificar imagen y preguntar por actualización
IMAGE_EXISTS=false
FORCE_REDEPLOY=false

# Ejecutar check_docker_image y capturar el código de salida
if check_docker_image; then
  # Imagen existe y está disponible
  IMAGE_EXISTS=true
  echo -e "\n${YELLOW}¿Quieres actualizar la imagen compilando los archivos? (y/N)${NC}"
  read -r response
  if [[ "$response" =~ ^[Yy]$ ]]; then
    echo "Actualizando imagen..."
    if build_docker_image; then
      echo -e "${GREEN}✓ Imagen actualizada exitosamente${NC}"
      
      # Forzar redeploy del servidor central con la nueva imagen
      echo -e "${YELLOW}🔄 Forzando redeploy del servidor central con nueva imagen...${NC}"
      echo -e "${BLUE}💡 Nota: Se eliminará el deployment existente para que use la nueva imagen${NC}"
      if kubectl get deployment servidor-central-deployment &> /dev/null; then
        echo "Eliminando deployment existente..."
        kubectl delete deployment servidor-central-deployment --ignore-not-found=true
        sleep 5
        echo -e "${GREEN}✓ Deployment eliminado, se recreará automáticamente con kubectl apply${NC}"
      else
        echo -e "${YELLOW}⚠ Deployment no encontrado, se creará con kubectl apply${NC}"
      fi
      
      # Marcar que se necesita redeploy para aplicar más tarde
      FORCE_REDEPLOY=true
    else
      echo -e "${RED}✗ Error al actualizar la imagen. Continuando con la imagen existente...${NC}"
    fi
  else
    echo "Continuando con la imagen existente..."
  fi
else
  # Imagen no existe
  echo -e "\n${YELLOW}La imagen servidor-central:latest no está disponible en el contexto de Minikube.${NC}"
  echo -e "${YELLOW}¿Quieres construir la imagen ahora? (Y/n)${NC}"
  read -r response
  if [[ "$response" =~ ^[Nn]$ ]]; then
    echo -e "${RED}No se puede continuar sin la imagen Docker. Saliendo...${NC}"
    exit 1
  else
    echo "Construyendo imagen..."
    if build_docker_image; then
      IMAGE_EXISTS=true
      echo -e "${GREEN}✓ Imagen construida exitosamente${NC}"
      
      # Forzar redeploy del servidor central con la nueva imagen
      echo -e "${YELLOW}🔄 Forzando redeploy del servidor central con nueva imagen...${NC}"
      echo -e "${BLUE}💡 Nota: Se eliminará el deployment existente para que use la nueva imagen${NC}"
      if kubectl get deployment servidor-central-deployment &> /dev/null; then
        echo "Eliminando deployment existente..."
        kubectl delete deployment servidor-central-deployment --ignore-not-found=true
        sleep 5
        echo -e "${GREEN}✓ Deployment eliminado, se recreará automáticamente con kubectl apply${NC}"
      else
        echo -e "${YELLOW}⚠ Deployment no encontrado, se creará con kubectl apply${NC}"
      fi
      
      # Marcar que se necesita redeploy para aplicar más tarde
      FORCE_REDEPLOY=true
    else
      echo -e "${RED}✗ Error al construir la imagen. Saliendo...${NC}"
      exit 1
    fi
  fi
fi

# Preguntar si continuar con el despliegue
if [ "$IMAGE_EXISTS" = true ]; then
  echo -e "\n${YELLOW}¿Quieres continuar con el despliegue en Minikube? (Y/n)${NC}"
  read -r response
  if [[ "$response" =~ ^[Nn]$ ]]; then
    echo "Despliegue cancelado por el usuario."
    exit 0
  fi
  echo "Continuando con el despliegue..."
fi

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

# Función para limpiar MetalLB completamente
cleanup_metallb() {
  echo "Limpiando MetalLB completamente..."
  kubectl delete namespace metallb-system --ignore-not-found=true
  kubectl delete -f https://raw.githubusercontent.com/metallb/metallb/v0.14.8/config/manifests/metallb-native.yaml --ignore-not-found=true
  sleep 10
}

# Función para verificar y corregir webhooks de MetalLB
fix_metallb_webhooks() {
  echo "Verificando webhooks de MetalLB..."
  
  # Verificar si hay webhooks duplicados o corruptos
  WEBHOOK_ERRORS=$(kubectl get validatingwebhookconfiguration 2>&1 | grep -i "connection refused\|timeout\|error" || true)
  
  if [ ! -z "$WEBHOOK_ERRORS" ]; then
    echo "Detectados problemas con webhooks de MetalLB. Limpiando..."
    cleanup_metallb
    
    echo "Reinstalando MetalLB..."
    kubectl apply -f https://raw.githubusercontent.com/metallb/metallb/v0.14.8/config/manifests/metallb-native.yaml --validate=false
    
    # Esperar a que los pods estén listos
    echo "Esperando a que los pods de MetalLB estén listos..."
    kubectl wait --namespace metallb-system \
      --for=condition=ready pod \
      --selector=app=metallb \
      --timeout=180s
      
    # Verificar que no hay pods duplicados
    CONTROLLER_PODS=$(kubectl get pods -n metallb-system -l app=metallb,component=controller --no-headers | wc -l)
    if [ $CONTROLLER_PODS -gt 1 ]; then
      echo "Detectados pods duplicados de controller. Limpiando..."
      kubectl delete pods -n metallb-system -l app=metallb,component=controller --force --grace-period=0
      sleep 10
    fi
  fi
}

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

# 4. Verificar y corregir webhooks de MetalLB antes de aplicar configuración
fix_metallb_webhooks

# 5. Aplicar metallb-config.yaml si el IPAddressPool no existe
echo "Comprobando IPAddressPool..."
if ! kubectl get ipaddresspools -n metallb-system custom-ip-pool &> /dev/null; then
  echo "Aplicando metallb-config.yaml..."
  
  # Verificar que el archivo existe en el directorio actual
  if [ -f "metallb-config.yaml" ]; then
    echo "Usando metallb-config.yaml del directorio actual"
    
    # Intentar aplicar con manejo de errores de webhook
    if ! kubectl apply -f metallb-config.yaml --validate=false; then
      echo "Error al aplicar metallb-config.yaml. Verificando webhooks..."
      
      # Verificar si el error es por webhooks
      if kubectl apply -f metallb-config.yaml 2>&1 | grep -q "webhook.*connection refused\|webhook.*timeout"; then
        echo "Detectado error de webhook. Limpiando y reinstalando MetalLB..."
        cleanup_metallb
        
        # Reinstalar MetalLB
        kubectl apply -f https://raw.githubusercontent.com/metallb/metallb/v0.14.8/config/manifests/metallb-native.yaml --validate=false
        
        # Esperar a que esté listo
        kubectl wait --namespace metallb-system \
          --for=condition=ready pod \
          --selector=app=metallb \
          --timeout=180s
        
        # Reintentar aplicar configuración
        echo "Reintentando aplicar metallb-config.yaml..."
        kubectl apply -f metallb-config.yaml --validate=false
      fi
    fi
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

# Función para verificar y corregir configuración de MetalLB
fix_metallb_config() {
  echo "Verificando configuración de MetalLB..."
  
  # Verificar si el namespace está en estado "terminating" o no existe
  NAMESPACE_STATUS=$(kubectl get namespace metallb-system -o jsonpath='{.status.phase}' 2>/dev/null || echo "NotFound")
  
  if [ "$NAMESPACE_STATUS" = "Terminating" ]; then
    echo -e "${RED}⚠ Namespace metallb-system está en estado 'Terminating'. Reinstalando MetalLB...${NC}"
    reinstall_metallb
    return $?
  elif [ "$NAMESPACE_STATUS" = "NotFound" ]; then
    echo -e "${YELLOW}⚠ Namespace metallb-system no encontrado. Reinstalando MetalLB...${NC}"
    reinstall_metallb
    return $?
  fi
  
  # Verificar si los pods de MetalLB están ejecutándose
  METALLB_PODS=$(kubectl get pods -n metallb-system --no-headers 2>/dev/null | wc -l)
  if [ "$METALLB_PODS" -eq 0 ]; then
    echo -e "${YELLOW}⚠ No se encontraron pods de MetalLB. Reinstalando...${NC}"
    reinstall_metallb
    return $?
  fi
  
  # Verificar si existen IPAddressPools
  if ! kubectl get ipaddresspools -n metallb-system &> /dev/null || [ "$(kubectl get ipaddresspools -n metallb-system --no-headers 2>/dev/null | wc -l)" -eq 0 ]; then
    echo -e "${YELLOW}⚠ No se encontraron IPAddressPools. Aplicando configuración...${NC}"
    
    # Intentar aplicar configuración normalmente primero
    if [ -f "metallb-config.yaml" ]; then
      echo "Aplicando metallb-config.yaml..."
      kubectl apply -f metallb-config.yaml --validate=false 2>/dev/null
      
      # Esperar a que se aplique
      sleep 5
      
      # Verificar que se aplicó correctamente
      if kubectl get ipaddresspools -n metallb-system &> /dev/null; then
        echo -e "${GREEN}✓ Configuración de MetalLB aplicada correctamente${NC}"
      else
        echo -e "${YELLOW}⚠ Error al aplicar configuración. Reinstalando MetalLB...${NC}"
        reinstall_metallb
        return $?
      fi
    else
      echo -e "${RED}✗ metallb-config.yaml no encontrado${NC}"
      return 1
    fi
  else
    echo -e "${GREEN}✓ IPAddressPools configurados correctamente${NC}"
  fi
}

# Verificación final del estado de MetalLB
echo -e "\n${GREEN}Verificando estado final de MetalLB...${NC}"
if kubectl get pods -n metallb-system &> /dev/null; then
  echo "Pods de MetalLB:"
  kubectl get pods -n metallb-system
  echo -e "\nIPAddressPools:"
  kubectl get ipaddresspools -n metallb-system || echo "No se encontraron IPAddressPools"
  echo -e "\nL2Advertisements:"
  kubectl get l2advertisements -n metallb-system || echo "No se encontraron L2Advertisements"
  
  # Verificar y corregir configuración
  fix_metallb_config
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

# 6. Configuración PSK simple (sin Keycloak)
echo "Usando configuración PSK simple..."

# Función para manejar redeploy forzado después de actualizar imagen
handle_forced_redeploy() {
  if [ "$FORCE_REDEPLOY" = true ]; then
    echo -e "\n${YELLOW}🔄 Procesando redeploy forzado del servidor central...${NC}"
    echo -e "${BLUE}💡 Esto asegura que el pod use la nueva imagen compilada${NC}"
    
    # Esperar a que el deployment se aplique
    echo "Esperando a que el nuevo deployment se aplique..."
    sleep 10
    
    # Verificar que el deployment existe
    if kubectl get deployment servidor-central-deployment &> /dev/null; then
      echo -e "${GREEN}✓ Nuevo deployment detectado${NC}"
      
      # Esperar a que el pod esté listo
      echo "Esperando a que el pod esté listo con la nueva imagen..."
      kubectl wait --for=condition=ready pod -l app=servidor-central --timeout=180s
      
      if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Pod con nueva imagen está listo${NC}"
        
        # Verificar que efectivamente está usando la nueva imagen
        NEW_POD_NAME=$(kubectl get pods -l app=servidor-central -o jsonpath="{.items[0].metadata.name}")
        if [ -n "$NEW_POD_NAME" ]; then
          POD_IMAGE=$(kubectl get pod $NEW_POD_NAME -o jsonpath="{.spec.containers[0].image}")
          echo -e "${GREEN}✓ Pod $NEW_POD_NAME está usando imagen: $POD_IMAGE${NC}"
          echo -e "${GREEN}✓ Redeploy forzado completado exitosamente${NC}"
          
          # Mostrar logs iniciales del nuevo pod
          echo -e "${BLUE}Logs iniciales del servidor central con nueva imagen:${NC}"
          kubectl logs $NEW_POD_NAME --tail=20
        fi
      else
        echo -e "${RED}✗ Error: Pod no está listo después de 3 minutos${NC}"
        echo "Verificando estado del deployment..."
        kubectl get deployment servidor-central-deployment
        kubectl get pods -l app=servidor-central
      fi
    else
      echo -e "${RED}✗ Error: Deployment no encontrado después de aplicar recursos${NC}"
    fi
    
    FORCE_REDEPLOY=false
  fi
}

# Función para verificar y corregir problemas del servidor central
fix_servidor_central() {
  echo "Verificando estado del servidor central..."
  
  # Verificar si el pod está en estado Pending
  PENDING_PODS=$(kubectl get pods -l app=servidor-central --no-headers | grep -c "Pending" || echo "0")
  
  if [ $PENDING_PODS -gt 0 ]; then
    echo "Detectados pods en estado Pending. Verificando recursos..."
    
    # Verificar recursos disponibles
    NODE_RESOURCES=$(kubectl describe nodes | grep -A 5 "Allocated resources" || echo "No se pueden obtener recursos")
    echo "Recursos del nodo:"
    echo "$NODE_RESOURCES"
    
    # Verificar eventos del pod
    POD_NAME=$(kubectl get pods -l app=servidor-central -o jsonpath="{.items[0].metadata.name}")
    if [ -n "$POD_NAME" ]; then
      echo "Eventos del pod $POD_NAME:"
      kubectl describe pod $POD_NAME | grep -A 10 "Events:"
    fi
    
    # Intentar eliminar y recrear el deployment
    echo "Eliminando deployment para recrearlo..."
    kubectl delete deployment servidor-central-deployment --ignore-not-found=true
    sleep 10
    
    # Recrear deployment
    if [ -d "kustomize" ]; then
      kubectl apply -k kustomize/
    elif [ -d "../kustomize" ]; then
      kubectl apply -k ../kustomize/
    fi
    
    # Esperar a que esté listo
    echo "Esperando a que el servidor central esté listo..."
    kubectl wait --for=condition=ready pod -l app=servidor-central --timeout=120s
  fi
}

# 7. Aplicar recursos de Kubernetes con Kustomize
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

# 7.1. Manejar redeploy forzado si se actualizó la imagen
handle_forced_redeploy

# Verificar que el servicio obtenga una IP externa
echo "Verificando asignación de IP externa..."
sleep 10
EXTERNAL_IP=$(kubectl get svc servidor-central-service -o jsonpath='{.status.loadBalancer.ingress[0].ip}' 2>/dev/null)
if [ -z "$EXTERNAL_IP" ] || [ "$EXTERNAL_IP" = "null" ]; then
  echo -e "${YELLOW}⚠ Servicio sin IP externa. Verificando configuración de MetalLB...${NC}"
  fix_metallb_config
  
  # Esperar y verificar nuevamente
  echo "Esperando asignación de IP..."
  sleep 15
  EXTERNAL_IP=$(kubectl get svc servidor-central-service -o jsonpath='{.status.loadBalancer.ingress[0].ip}' 2>/dev/null)
  if [ -z "$EXTERNAL_IP" ] || [ "$EXTERNAL_IP" = "null" ]; then
    echo -e "${RED}✗ No se pudo asignar IP externa al servicio${NC}"
  else
    echo -e "${GREEN}✓ IP externa asignada: $EXTERNAL_IP${NC}"
  fi
else
  echo -e "${GREEN}✓ IP externa asignada: $EXTERNAL_IP${NC}"
fi

# 8. Verificar y corregir problemas del servidor central
fix_servidor_central

# 9. Verificar el estado
echo -e "\n${GREEN}Verificando estado del despliegue...${NC}"
echo "Servicios:"
kubectl get services
echo -e "\nPods:"
kubectl get pods
echo -e "\nHPA:"
kubectl get hpa

# 10. Mostrar información de servicios desplegados
echo -e "\n${GREEN}=== INFORMACIÓN DE SERVICIOS DESPLEGADOS ===${NC}"

# Mostrar IP del servidor central
if [ -n "$EXTERNAL_IP" ] && [ "$EXTERNAL_IP" != "null" ]; then
  echo -e "${GREEN}✓ Servidor Central disponible en: $EXTERNAL_IP:5684${NC}"
else
  echo -e "${RED}✗ Servidor Central sin IP externa${NC}"
fi

echo -e "${GREEN}✓ Configuración PSK simple activada${NC}"

echo -e "\n${GREEN}=== CONFIGURACIÓN COMPLETADA ===${NC}"
echo "Sistema configurado con PSK simple (sin Keycloak)."

echo -e "\n${GREEN}=== FUNCIONALIDAD MEJORADA ===${NC}"
echo -e "${BLUE}💡 Redeploy Automático Activado:${NC}"
echo "  • Cuando se construye/actualiza la imagen del servidor central"
echo "  • Se elimina automáticamente el deployment existente"
echo "  • Se fuerza la creación de un nuevo pod con la imagen actualizada"
echo "  • Ya no necesitas hacer 'kubectl delete deployment' manualmente"
echo ""
echo -e "${BLUE}🔧 Autorecuperación de MetalLB:${NC}"
echo "  • Detecta automáticamente cuando MetalLB está en mal estado"
echo "  • Elimina y reinstala MetalLB completamente si es necesario"
echo "  • Resuelve problemas de namespace 'Terminating' automáticamente"
echo "  • No más errores manuales de configuración de MetalLB"

# 11. Verificar logs del pod
sleep 5
echo -e "\n${GREEN}Mostrando logs del servidor central...${NC}"

POD_NAME=$(kubectl get pods -l app=servidor-central -o jsonpath="{.items[0].metadata.name}")
if [ -n "$POD_NAME" ]; then
  POD_STATUS=$(kubectl get pod $POD_NAME -o jsonpath="{.status.phase}")
  
  if [ "$POD_STATUS" = "Running" ]; then
    echo -e "${GREEN}Pod $POD_NAME está ejecutándose. Mostrando logs...${NC}"
    kubectl logs -f $POD_NAME
  elif [ "$POD_STATUS" = "Pending" ]; then
    echo -e "${YELLOW}Pod $POD_NAME está en estado Pending. Mostrando eventos...${NC}"
    kubectl describe pod $POD_NAME | grep -A 20 "Events:"
  elif [ "$POD_STATUS" = "Failed" ]; then
    echo -e "${RED}Pod $POD_NAME falló. Mostrando logs de error...${NC}"
    kubectl logs $POD_NAME --previous || kubectl logs $POD_NAME
  else
    echo -e "${YELLOW}Pod $POD_NAME está en estado: $POD_STATUS${NC}"
    kubectl describe pod $POD_NAME
  fi
else
  echo -e "${RED}No se encontró el pod del servidor-central.${NC}"
  echo "Verificando deployments..."
  kubectl get deployments
  echo "Verificando eventos..."
  kubectl get events --sort-by='.lastTimestamp'
fi