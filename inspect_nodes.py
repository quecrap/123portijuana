import struct, json

with open(r'c:\Users\PEPITO\Downloads\sistema binaryfile.glb', 'rb') as f:
    magic, ver, length = struct.unpack('<4sII', f.read(12))
    chunk_len, chunk_type = struct.unpack('<II', f.read(8))
    d = json.loads(f.read(chunk_len).decode('utf-8'))

nodes = d.get('nodes', [])
for i, n in enumerate(nodes):
    name = n.get('name', '')
    if any(k in name for k in ['Electro', 'Conico', 'Conexiones', '12_Bomba']):
        print(f"Node {i}: {name}")
        children = n.get('children', [])
        for c in children:
            cn = nodes[c]
            mesh_idx = cn.get('mesh', None)
            mesh_name = d['meshes'][mesh_idx].get('name', '') if mesh_idx is not None else ''
            print(f"   child {c}: {cn.get('name', '')} mesh={mesh_idx} ({mesh_name}) trans={cn.get('translation', '')}")
