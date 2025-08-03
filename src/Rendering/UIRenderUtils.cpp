#include "Rendering/UIRenderUtils.h"

static ImGuiTreeNodeFlags s_treeNodeFlags = ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_SpanFullWidth
                                            | ImGuiTreeNodeFlags_OpenOnArrow
                                            | ImGuiTreeNodeFlags_OpenOnDoubleClick;

namespace gore::renderer
{
void RenderRootGameObject(const GameObject* rootNode)
{
    if (rootNode == nullptr)
        return;

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    if (ImGui::TreeNodeEx(rootNode->GetName().c_str(), s_treeNodeFlags))
    {
        ImGui::TreePop();
    }
}
} // namespace gore::renderer