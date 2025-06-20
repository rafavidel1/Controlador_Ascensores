#!/bin/bash

# Script para limpiar el directorio temporal de build
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TEMP_BUILD_DIR="$PROJECT_ROOT/temp-build-tests"
REPORTS_DIR="$SCRIPT_DIR/resultados_reportes"

echo "🧹 Limpiador de directorio temporal de build"
echo "================================================"
echo "📁 Proyecto: $PROJECT_ROOT"
echo "🗑️  Directorio temporal: $TEMP_BUILD_DIR"
echo "📊 Directorio de reportes: $REPORTS_DIR (se mantiene)"
echo ""

if [ -d "$TEMP_BUILD_DIR" ]; then
    echo "🔍 Directorio temporal encontrado. Eliminando..."
    rm -rf "$TEMP_BUILD_DIR"
    echo "✅ Directorio temporal eliminado exitosamente"
else
    echo "ℹ️  No hay directorio temporal que limpiar"
fi

echo ""
if [ -d "$REPORTS_DIR" ]; then
    echo "📊 Reportes conservados en: $REPORTS_DIR"
    echo "   Archivos disponibles:"
    find "$REPORTS_DIR" -name "*.txt" -o -name "*.json" -o -name "*.xml" | while read -r file; do
        echo "   - $(basename "$file")"
    done
else
    echo "ℹ️  No hay reportes generados aún"
fi

echo ""
echo "✅ Limpieza completada" 