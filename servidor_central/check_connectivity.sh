#!/bin/bash

# Colores para la salida
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # Sin color

echo -e "${BLUE}🔍 Comprobación de Conectividad - Kubernetes & MetalLB${NC}"
echo "=========================================================="

# Contador de problemas
PROBLEMS=0
TOTAL_CHECKS=0

# Función para mostrar resultado
show_result() {
  TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
  if [ $1 -eq 0 ]; then
    echo -e "${GREEN}✓ $2${NC}"
  else
    echo -e "${RED}✗ $2${NC}"
    PROBLEMS=$((PROBLEMS + 1))
  fi
}

# Función para mostrar información adicional
show_info() {
  echo -e "${BLUE}ℹ️  $1${NC}"
}

echo -e "\n${YELLOW}1. Comprobando Minikube...${NC}"
# Verificar si minikube está corriendo
if minikube status | grep -q "Running"; then
  show_result 0 "Minikube está corriendo"
  show_info "Estado completo: $(minikube status | head -1)"
else
  show_result 1 "Minikube NO está corriendo"
fi

echo -e "\n${YELLOW}2. Comprobando conectividad con API de Kubernetes...${NC}"
# Verificar conectividad con la API
if kubectl cluster-info &> /dev/null; then
  show_result 0 "API de Kubernetes accesible"
  show_info "Endpoint: $(kubectl cluster-info | grep 'Kubernetes control plane' | cut -d' ' -f7)"
else
  show_result 1 "API de Kubernetes NO accesible"
fi

echo -e "\n${YELLOW}3. Comprobando addon de MetalLB...${NC}"
# Verificar si MetalLB está habilitado como addon
if minikube addons list | grep metallb | grep -q enabled; then
  show_result 0 "MetalLB addon habilitado"
else
  show_result 1 "MetalLB addon NO habilitado"
fi

echo -e "\n${YELLOW}4. Comprobando CRDs de MetalLB...${NC}"
# Verificar si los CRDs están instalados
if kubectl get crds | grep -q ipaddresspools.metallb.io; then
  show_result 0 "CRDs de MetalLB instalados"
  show_info "CRDs encontrados: $(kubectl get crds | grep metallb | wc -l)"
else
  show_result 1 "CRDs de MetalLB NO instalados"
fi

echo -e "\n${YELLOW}5. Comprobando namespace de MetalLB...${NC}"
# Verificar si el namespace existe
if kubectl get namespace metallb-system &> /dev/null; then
  show_result 0 "Namespace metallb-system existe"
else
  show_result 1 "Namespace metallb-system NO existe"
fi

echo -e "\n${YELLOW}6. Comprobando pods de MetalLB...${NC}"
# Verificar pods de MetalLB
if kubectl get pods -n metallb-system &> /dev/null; then
  POD_COUNT=$(kubectl get pods -n metallb-system --no-headers | wc -l)
  RUNNING_PODS=$(kubectl get pods -n metallb-system --no-headers | grep -c "Running")
  
  if [ $POD_COUNT -gt 0 ]; then
    show_result 0 "Pods de MetalLB encontrados ($POD_COUNT total, $RUNNING_PODS corriendo)"
    
    # Mostrar estado de cada pod
    echo -e "${BLUE}  Estado de pods:${NC}"
    kubectl get pods -n metallb-system --no-headers | while read line; do
      POD_NAME=$(echo "$line" | awk '{print $1}')
      POD_STATUS=$(echo "$line" | awk '{print $3}')
      if [ "$POD_STATUS" = "Running" ]; then
        echo -e "    ${GREEN}✓ $POD_NAME ($POD_STATUS)${NC}"
      else
        echo -e "    ${RED}✗ $POD_NAME ($POD_STATUS)${NC}"
      fi
    done
  else
    show_result 1 "No hay pods de MetalLB"
  fi
else
  show_result 1 "No se pueden obtener pods de MetalLB"
fi

echo -e "\n${YELLOW}7. Comprobando IPAddressPools...${NC}"
# Verificar IPAddressPools
if kubectl get ipaddresspools -n metallb-system &> /dev/null; then
  POOL_COUNT=$(kubectl get ipaddresspools -n metallb-system --no-headers | wc -l)
  if [ $POOL_COUNT -gt 0 ]; then
    show_result 0 "IPAddressPools encontrados ($POOL_COUNT)"
    kubectl get ipaddresspools -n metallb-system --no-headers | while read line; do
      echo -e "    ${GREEN}✓ $line${NC}"
    done
  else
    show_result 1 "No hay IPAddressPools configurados"
  fi
else
  show_result 1 "No se pueden obtener IPAddressPools"
fi

echo -e "\n${YELLOW}8. Comprobando L2Advertisements...${NC}"
# Verificar L2Advertisements
if kubectl get l2advertisements -n metallb-system &> /dev/null; then
  L2_COUNT=$(kubectl get l2advertisements -n metallb-system --no-headers | wc -l)
  if [ $L2_COUNT -gt 0 ]; then
    show_result 0 "L2Advertisements encontrados ($L2_COUNT)"
    kubectl get l2advertisements -n metallb-system --no-headers | while read line; do
      echo -e "    ${GREEN}✓ $line${NC}"
    done
  else
    show_result 1 "No hay L2Advertisements configurados"
  fi
else
  show_result 1 "No se pueden obtener L2Advertisements"
fi

echo -e "\n${YELLOW}9. Comprobando servicios LoadBalancer...${NC}"
# Verificar servicios LoadBalancer
LB_SERVICES=$(kubectl get services --all-namespaces -o jsonpath='{.items[?(@.spec.type=="LoadBalancer")].metadata.name}' 2>/dev/null | wc -w)
if [ $LB_SERVICES -gt 0 ]; then
  show_result 0 "Servicios LoadBalancer encontrados ($LB_SERVICES)"
  kubectl get services --all-namespaces -o jsonpath='{range .items[?(@.spec.type=="LoadBalancer")]}{.metadata.namespace}/{.metadata.name}:{.status.loadBalancer.ingress[0].ip}{"\n"}{end}' 2>/dev/null | while read line; do
    if [ -n "$line" ]; then
      echo -e "    ${GREEN}✓ $line${NC}"
    fi
  done
else
  show_result 1 "No hay servicios LoadBalancer"
fi

echo -e "\n${YELLOW}10. Comprobando conectividad de red...${NC}"
# Verificar conectividad básica de red
if ping -c 1 8.8.8.8 &> /dev/null; then
  show_result 0 "Conectividad de red básica OK"
else
  show_result 1 "Problemas de conectividad de red"
fi

# Resumen final
echo -e "\n${BLUE}📊 RESUMEN DE CONECTIVIDAD${NC}"
echo "================================"
echo -e "Total de comprobaciones: ${BLUE}$TOTAL_CHECKS${NC}"
echo -e "Problemas encontrados: ${RED}$PROBLEMS${NC}"

if [ $PROBLEMS -eq 0 ]; then
  echo -e "\n${GREEN}🎉 ¡Todo está funcionando correctamente!${NC}"
  exit 0
elif [ $PROBLEMS -le 3 ]; then
  echo -e "\n${YELLOW}⚠️  Algunos problemas menores detectados${NC}"
  exit 1
else
  echo -e "\n${RED}❌ Múltiples problemas detectados${NC}"
  exit 2
fi 