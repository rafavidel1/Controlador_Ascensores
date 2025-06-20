# elevator_state_t

## Properties
Name | Type | Description | Notes
------------ | ------------- | ------------- | -------------
**id_ascensor** | **char \*** | Identificador único del ascensor (formato EdificioIDAnumero). | 
**piso_actual** | **int** | Piso actual del ascensor (0 &#x3D; planta baja). | 
**estado_puerta** | **door_state_t \*** |  | 
**disponible** | **int** | Indica si el ascensor está disponible para nuevas tareas. | 
**tarea_actual_id** | **char \*** | ID de la tarea actual asignada por el servidor central. | [optional] 
**destino_actual** | **int** | Piso destino actual. Valor -1 o null indica sin destino asignado. | [optional] 

[[Back to Model list]](../README.md#documentation-for-models) [[Back to API list]](../README.md#documentation-for-api-endpoints) [[Back to README]](../README.md)


