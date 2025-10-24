import os

def can_build(env, platform):
    return True

def configure(env):
    # Vérifier que NINTENDO_SDK_ROOT est défini
    if env['platform'] == "nx":
        try:
            sdk_root = env["NINTENDO_SDK_ROOT"]
        except KeyError:
            print("Erreur : NINTENDO_SDK_ROOT n'est pas défini dans l'environnement SCons.")
            Exit(1)
        
        # Construit le chemin vers le dossier Include
        sdk_include_path = os.path.join(sdk_root, 'Include')
        print("Utilisation du chemin d'inclusion Nintendo SDK :", sdk_include_path)
    
        if not os.path.exists(sdk_include_path):
            print("Erreur : Le dossier d'inclusion Nintendo SDK n'existe pas :", sdk_include_path)
            sys.exit(1)
    
        # Ajoute ce chemin aux chemins d'inclusion
        env.Prepend(CPPPATH=[sdk_include_path])
