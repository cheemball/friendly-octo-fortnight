# Supramundo (mod de Geode para Geometry Dash)

## Compilar
1. Instala el Geode SDK + CLI y define `GEODE_SDK`.
2. `geode build` (o sube a GitHub: el workflow `.github/workflows/build.yml` compila para Windows/macOS/Android).
3. Copia el `.geode` generado a la carpeta `geode/mods` del juego.

## Para GD 2.209
Cuando Geode soporte 2.209: cambia `"gd"` y `"geode"` en `mod.json` a las versiones nuevas
y cambia `tuusuario`/`TuUsuario` por tu nombre. Si algun binding cambio (por ejemplo
`m_levelManagerDelegate`, `isPlatformer`, `m_demon`), ajusta `src/SupraLayer.cpp`.

## Contenido
- 5 biomas, 5 puertas con coste en puntos, 3 Torres Demoniacas, 2 tiendas, 5 cofres ocultos, 9 monstruos, The King.
- Niveles: se piden los recientes (y destacados para torres), solo plataforma, emparejados por palabras clave del nombre.
- Recompensas al completar niveles (se comprueba al volver al mapa).
