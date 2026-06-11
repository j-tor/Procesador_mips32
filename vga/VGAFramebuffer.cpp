#include "VGAFramebuffer.h"

const VGAFramebuffer::CellBuffer& VGAFramebuffer::swapAndGetFront() {
    if (m_frameReady.exchange(false, std::memory_order_acq_rel)) {
        // Safe: swap only happens when reader initiates it
        m_frontIndex.store(
            1 - m_frontIndex.load(std::memory_order_relaxed),
            std::memory_order_release
        );
    }
    return m_buffers[m_frontIndex.load(std::memory_order_acquire)];
}

void VGAFramebuffer::clear(uint8_t fg, uint8_t bg) noexcept {
    const uint16_t entry = makeEntry(' ', fg, bg);
    int back = 1 - m_frontIndex.load(std::memory_order_acquire);
    auto& backBuffer = m_buffers[back];

    backBuffer.fill(entry);
}

void VGAFramebuffer::writePixel(size_t row, size_t col, uint8_t ch, uint8_t fg, uint8_t bg) noexcept {
    if (row >= VGA_ROWS || col >= VGA_COLS) {
        return;
    }

    int back = 1 - m_frontIndex.load(std::memory_order_acquire);
    auto& backBuffer = m_buffers[back];

    const int cellIdx = row * static_cast<int>(VGA_COLS) + col;
    backBuffer[cellIdx] = makeEntry(ch, fg, bg);
}