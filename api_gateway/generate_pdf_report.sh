#!/bin/bash

# generate_pdf_report.sh - Convierte logs de Markdown a PDF
# Uso: ./generate_pdf_report.sh [archivo.md]

set -e

# Configuración
LOGS_DIR="logs"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Función para mostrar ayuda
show_help() {
    echo "Uso: $0 [OPCIONES] [archivo.md]"
    echo ""
    echo "Convierte archivos de log Markdown a PDF usando pandoc"
    echo ""
    echo "OPCIONES:"
    echo "  -h, --help     Muestra esta ayuda"
    echo "  -l, --latest   Convierte el archivo más reciente"
    echo "  -a, --all      Convierte todos los archivos .md en logs/"
    echo ""
    echo "EJEMPLOS:"
    echo "  $0 logs/2025-01-17/ejecucion_18-05-57.md"
    echo "  $0 --latest"
    echo "  $0 --all"
}

# Función para verificar dependencias
check_dependencies() {
    if ! command -v pandoc &> /dev/null; then
        echo "ERROR: pandoc no está instalado"
        echo "Instala pandoc con: sudo apt-get install pandoc texlive-xetex"
        exit 1
    fi
    
    if ! command -v xelatex &> /dev/null; then
        echo "ERROR: xelatex no está disponible"
        echo "Instala texlive-xetex con: sudo apt-get install texlive-xetex"
        exit 1
    fi
}

# Función para convertir un archivo MD a PDF
convert_to_pdf() {
    local md_file="$1"
    local base_name=$(basename "$md_file" .md)
    local dir_name=$(dirname "$md_file")
    local pdf_file="$dir_name/${base_name}.pdf"
    
    echo "Convirtiendo: $md_file -> $pdf_file"
    
    # Verificar que el archivo existe
    if [ ! -f "$md_file" ]; then
        echo "ERROR: El archivo $md_file no existe"
        return 1
    fi
    
    # Convertir con pandoc usando configuración optimizada
    pandoc "$md_file" \
        -o "$pdf_file" \
        --pdf-engine=xelatex \
        --variable mainfont="DejaVu Sans" \
        --variable monofont="DejaVu Sans Mono" \
        --variable geometry:margin=2cm \
        --variable fontsize=11pt \
        --variable documentclass=article \
        --variable colorlinks=true \
        --variable linkcolor=blue \
        --variable urlcolor=blue \
        --variable toccolor=black \
        --table-of-contents \
        --number-sections \
        --highlight-style=tango \
        --standalone
    
    if [ $? -eq 0 ]; then
        echo "✓ PDF generado exitosamente: $pdf_file"
        
        # Mostrar información del archivo
        if command -v ls &> /dev/null; then
            echo "  Tamaño: $(ls -lh "$pdf_file" | awk '{print $5}')"
        fi
        
        return 0
    else
        echo "✗ Error al generar PDF"
        return 1
    fi
}

# Función para encontrar el archivo más reciente
find_latest_log() {
    find "$LOGS_DIR" -name "*.md" -type f -printf '%T@ %p\n' 2>/dev/null | \
    sort -n | tail -1 | cut -d' ' -f2-
}

# Función principal
main() {
    local convert_latest=false
    local convert_all=false
    local target_file=""
    
    # Procesar argumentos
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -l|--latest)
                convert_latest=true
                shift
                ;;
            -a|--all)
                convert_all=true
                shift
                ;;
            -*)
                echo "ERROR: Opción desconocida $1"
                show_help
                exit 1
                ;;
            *)
                target_file="$1"
                shift
                ;;
        esac
    done
    
    # Verificar dependencias
    check_dependencies
    
    # Cambiar al directorio del script
    cd "$SCRIPT_DIR"
    
    # Determinar la ruta correcta de logs
    if [ -d "logs" ]; then
        LOGS_DIR="logs"
        echo "Usando directorio: logs/"
    elif [ -d "../api_gateway/logs" ]; then
        LOGS_DIR="../api_gateway/logs"
        echo "Usando directorio: ../api_gateway/logs/"
    elif [ -d "api_gateway/logs" ]; then
        LOGS_DIR="api_gateway/logs"
        echo "Usando directorio: api_gateway/logs/"
    else
        echo "ERROR: No se encontró directorio de logs"
        echo "Buscado en:"
        echo "  - logs/"
        echo "  - ../api_gateway/logs/"
        echo "  - api_gateway/logs/"
        exit 1
    fi
    
    # Verificar que existe el directorio de logs
    if [ ! -d "$LOGS_DIR" ]; then
        echo "ERROR: El directorio $LOGS_DIR no existe"
        exit 1
    fi
    
    # Ejecutar según las opciones
    if [ "$convert_all" = true ]; then
        echo "Convirtiendo todos los archivos de log..."
        local count=0
        local success=0
        
        while IFS= read -r -d '' file; do
            ((count++))
            if convert_to_pdf "$file"; then
                ((success++))
            fi
        done < <(find "$LOGS_DIR" -name "*.md" -type f -print0 2>/dev/null)
        
        echo ""
        echo "Resumen: $success/$count archivos convertidos exitosamente"
        
    elif [ "$convert_latest" = true ]; then
        echo "Buscando el archivo de log más reciente..."
        local latest_file=$(find_latest_log)
        
        if [ -z "$latest_file" ]; then
            echo "ERROR: No se encontraron archivos .md en $LOGS_DIR"
            exit 1
        fi
        
        echo "Archivo más reciente: $latest_file"
        convert_to_pdf "$latest_file"
        
    elif [ -n "$target_file" ]; then
        convert_to_pdf "$target_file"
        
    else
        echo "ERROR: Debes especificar un archivo o usar --latest/--all"
        show_help
        exit 1
    fi
}

# Ejecutar función principal
main "$@" 