import bpy
#blenderに登録するアドオン情報
bl_info = {
    "name": "レベルエディタ",
    "author": "Hiroto Mikami",
    "version": (4, 4),
    "blender": (4, 4, 0),
    "location": "",
    "description": "レベルエディタ",
    "warning": "",
    "waki_url": "",
    "tracler_url": "",
    "category":"Object"
}

#アドオン有効化時コールバック
def register():
    print("レベルエディタが有効化されました")


#アドオン無効化時コールバック
def unregister():
    print("レベルエディタが無効化されました")
    
    
if __name__ == "__main__":
    register();