#include "CommandRing.h"

namespace gore::gfx
{
CommandRingElement RequestNextCommandElement(CommandRing* ring, bool cyclePool, uint32_t cmdBufferCount)
{
    if (cyclePool)
    {
        ring->currentPoolIndex      = (ring->currentPoolIndex + 1) % ring->poolCount;
        ring->currentCmdIndex       = 0;
        ring->currentFenceIndex     = 0;
        ring->currentSemaphoreIndex = 0;
    }

    if (ring->currentCmdIndex + cmdBufferCount > ring->cmdBufferCountPerPool)
    {
        LOG_STREAM(FATAL) << "Failed to request next command element: command buffer count exceeds the limit of the pool" << std::endl;
        return {};
    }

    ring->currentCmdIndex += cmdBufferCount;
    ring->currentSemaphoreIndex += 1;
    ring->currentFenceIndex += 1;

    return {
        ring->cmdPools[ring->currentPoolIndex], 
        &ring->cmdBuffers[ring->currentPoolIndex][ring->currentCmdIndex - cmdBufferCount], 
        ring->fences[ring->currentPoolIndex][ring->currentFenceIndex - 1], 
        ring->semaphores[ring->currentPoolIndex][ring->currentSemaphoreIndex - 1], 
        cmdBufferCount};
}
} // namespace gore::gfx