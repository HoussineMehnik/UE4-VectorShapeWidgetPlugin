#include "VectorShapeTypes.h"



#ifdef WITH_EDITOR
FVectorShapeEditorDelegates::FOnCreateVectorMesh FVectorShapeEditorDelegates::OnCreateVectorMeshDelegate;
FVectorShapeEditorDelegates::FOnVectorDataAssetChanged FVectorShapeEditorDelegates::OnVectorDataAssetChangedDelegate;

#endif // WITH_EDITOR


DEFINE_LOG_CATEGORY(LogVectorShape);