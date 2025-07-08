#!/bin/bash

# Script súper simple para generar documentación Doxygen del API Gateway
# Sin complicaciones, sin errores

echo "📚 Generando documentación Doxygen del API Gateway..."

# Verificar Doxygen
if ! command -v doxygen &> /dev/null; then
    echo "❌ Error: Doxygen no está instalado"
    echo "Instala con: sudo apt-get install doxygen"
    exit 1
fi

# Crear directorio
mkdir -p docs

# Limpiar anterior
if [ -d "docs/latex" ]; then
    echo "🗑️  Limpiando documentación anterior..."
    rm -rf docs/latex
fi

# Generar con configuración simple
echo "📖 Generando documentación..."
doxygen Doxyfile_simple

if [ $? -eq 0 ]; then
    echo "✅ Documentación generada exitosamente"
    
    if [ -f "docs/latex/refman.tex" ]; then
        echo "📄 Archivo LaTeX creado: docs/latex/refman.tex"
        
        # Intentar compilar PDF
        if command -v pdflatex &> /dev/null; then
            echo "📄 Compilando PDF..."
            cd docs/latex
            pdflatex -interaction=nonstopmode refman.tex > /dev/null 2>&1
            
            if [ -f "refman.pdf" ]; then
                echo "✅ PDF generado: docs/latex/refman.pdf"
            else
                echo "⚠️  PDF no generado, pero LaTeX está listo"
            fi
            cd ../..
        else
            echo "⚠️  pdflatex no disponible"
        fi
    else
        echo "❌ No se generó refman.tex"
    fi
else
    echo "❌ Error al generar documentación"
    exit 1
fi

echo ""
echo "📊 Documentación generada:"
echo "  📄 LaTeX: docs/latex/refman.tex"
if [ -f "docs/latex/refman.pdf" ]; then
    echo "  ✅ PDF: docs/latex/refman.pdf"
fi
echo ""
echo "🎉 Listo para TFG"
echo "💡 Para usar: copia refman.tex a tu documento LaTeX" 