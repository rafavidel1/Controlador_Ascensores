#!/usr/bin/env python3
"""
Script para añadir solicitudes de emergencia al archivo simulation_data.json

Este script añade automáticamente 2 solicitudes de emergencia diferentes
a cada edificio en el archivo de simulación para probar el endpoint
/llamada_emergencia implementado en el servidor central.

Tipos de emergencia disponibles:
- EMERGENCY_STOP: Parada de emergencia
- POWER_FAILURE: Fallo de alimentación  
- PEOPLE_TRAPPED: Personas atrapadas
- MECHANICAL_FAILURE: Fallo mecánico
- FIRE_ALARM: Alarma de incendio
"""

import json
import random
from datetime import datetime, timedelta

def generate_emergency_request(building_id, emergency_type, elevator_index, current_floor):
    """
    Genera una solicitud de emergencia con datos realistas
    
    Args:
        building_id: ID del edificio (ej: "E001")
        emergency_type: Tipo de emergencia 
        elevator_index: Índice del ascensor (0-2)
        current_floor: Piso actual del ascensor
    
    Returns:
        dict: Solicitud de emergencia formateada
    """
    # Generar timestamp realista (dentro de las próximas horas)
    base_time = datetime.now()
    random_minutes = random.randint(10, 300)  # Entre 10 minutos y 5 horas
    timestamp = (base_time + timedelta(minutes=random_minutes)).isoformat()
    
    # Descripciones específicas por tipo de emergencia
    descriptions = {
        "EMERGENCY_STOP": "Botón de emergencia activado por usuario",
        "POWER_FAILURE": "Pérdida de suministro eléctrico principal", 
        "PEOPLE_TRAPPED": "Personas atrapadas entre pisos",
        "MECHANICAL_FAILURE": "Fallo en sistema de tracción",
        "FIRE_ALARM": "Detección de humo en shaft del ascensor"
    }
    
    return {
        "tipo": "llamada_emergencia",
        "id_edificio": building_id,
        "ascensor_id_emergencia": f"ASC_{building_id}_{elevator_index:02d}",
        "tipo_emergencia": emergency_type,
        "piso_actual_emergencia": current_floor,
        "timestamp_emergencia": timestamp,
        "descripcion_emergencia": descriptions.get(emergency_type, "Emergencia no especificada"),
        "elevadores_estado": [
            {
                "id_ascensor": f"ASC_{building_id}_00",
                "disponible": elevator_index != 0,
                "piso_actual": random.randint(0, 9)
            },
            {
                "id_ascensor": f"ASC_{building_id}_01", 
                "disponible": elevator_index != 1,
                "piso_actual": random.randint(0, 9)
            },
            {
                "id_ascensor": f"ASC_{building_id}_02",
                "disponible": elevator_index != 2,
                "piso_actual": random.randint(0, 9)
            }
        ]
    }

def add_emergency_requests_to_simulation():
    """
    Añade 2 solicitudes de emergencia a cada edificio en simulation_data.json
    """
    
    # Tipos de emergencia disponibles
    emergency_types = [
        "EMERGENCY_STOP",
        "POWER_FAILURE", 
        "PEOPLE_TRAPPED",
        "MECHANICAL_FAILURE",
        "FIRE_ALARM"
    ]
    
    print("🚨 Cargando simulation_data.json...")
    
    # Leer archivo original
    try:
        with open('api_gateway/simulation_data.json', 'r', encoding='utf-8') as f:
            data = json.load(f)
    except FileNotFoundError:
        print("❌ Error: No se encuentra el archivo api_gateway/simulation_data.json")
        return False
    except json.JSONDecodeError as e:
        print(f"❌ Error parseando JSON: {e}")
        return False
    
    print(f"✅ Archivo cargado. Edificios encontrados: {len(data['edificios'])}")
    
    # Contador de emergencias añadidas
    total_emergencies_added = 0
    
    # Procesar cada edificio
    for building in data['edificios']:
        building_id = building['id_edificio']
        
        print(f"🏢 Procesando edificio {building_id}...")
        
        # Generar 2 emergencias diferentes para este edificio
        emergency_1_type = random.choice(emergency_types)
        emergency_2_type = random.choice([t for t in emergency_types if t != emergency_1_type])
        
        # Emergencia 1: Ascensor aleatorio, piso aleatorio
        elevator_1 = random.randint(0, 2)
        floor_1 = random.randint(0, 9)
        
        # Emergencia 2: Ascensor diferente, piso diferente  
        elevator_2 = random.choice([i for i in range(3) if i != elevator_1])
        floor_2 = random.choice([f for f in range(10) if f != floor_1])
        
        # Generar las solicitudes de emergencia
        emergency_request_1 = generate_emergency_request(
            building_id, emergency_1_type, elevator_1, floor_1
        )
        
        emergency_request_2 = generate_emergency_request(
            building_id, emergency_2_type, elevator_2, floor_2
        )
        
        # Añadir las emergencias al edificio
        building['peticiones'].extend([emergency_request_1, emergency_request_2])
        
        total_emergencies_added += 2
        
        print(f"  ✅ Añadidas 2 emergencias: {emergency_1_type} (ASC_{elevator_1}, piso {floor_1}), "
              f"{emergency_2_type} (ASC_{elevator_2}, piso {floor_2})")
    
    print(f"\n🚨 Total de emergencias añadidas: {total_emergencies_added}")
    
    # Guardar archivo actualizado
    print("💾 Guardando simulation_data.json actualizado...")
    
    try:
        with open('api_gateway/simulation_data.json', 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        
        print("✅ Archivo guardado exitosamente!")
        
        # Mostrar estadísticas
        print(f"\n📊 Estadísticas finales:")
        print(f"   • Edificios procesados: {len(data['edificios'])}")
        print(f"   • Emergencias por edificio: 2")
        print(f"   • Total emergencias añadidas: {total_emergencies_added}")
        
        # Mostrar distribución de tipos de emergencia
        emergency_counts = {}
        for building in data['edificios']:
            for request in building['peticiones']:
                if request['tipo'] == 'llamada_emergencia':
                    emergency_type = request['tipo_emergencia']
                    emergency_counts[emergency_type] = emergency_counts.get(emergency_type, 0) + 1
        
        print(f"\n🔍 Distribución de tipos de emergencia:")
        for emergency_type, count in sorted(emergency_counts.items()):
            print(f"   • {emergency_type}: {count} solicitudes")
        
        return True
        
    except Exception as e:
        print(f"❌ Error guardando archivo: {e}")
        return False

if __name__ == "__main__":
    print("🚨 Script para añadir solicitudes de emergencia")
    print("=" * 50)
    
    success = add_emergency_requests_to_simulation()
    
    if success:
        print("\n✅ ¡Proceso completado exitosamente!")
        print("🔧 Ahora puedes ejecutar las pruebas con las solicitudes de emergencia")
    else:
        print("\n❌ ¡Proceso falló!")
        print("🔍 Revisa los errores mostrados arriba") 