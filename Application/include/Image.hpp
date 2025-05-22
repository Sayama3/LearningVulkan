//
// Created by ianpo on 22/05/2025.
//

#pragma once

#include <cstdint>
#include <stdexcept>
#include <exception>

namespace Imagine::Core {

	template<typename PixelType = uint8_t>
	class Image {
	public:
		static constexpr uint64_t PixelSize = sizeof(PixelType);
	public:
		Image() = default;
		~Image() {
			Release();
		}
	public:
		void Set(const uint64_t new_width, const uint64_t new_height, const uint8_t new_channels) {
			Release();
			m_Height = new_height;
			m_Width = new_width;
			m_Channels = new_channels;
			m_Pixels = reinterpret_cast<PixelType*>(calloc(Count(), PixelSize));
		}

		void Set(const PixelType*& pixels, const uint64_t new_width, const uint64_t new_height, const uint8_t new_channels) {
			Release();
			m_Height = new_height;
			m_Width = new_width;
			m_Channels = new_channels;
			m_Pixels = reinterpret_cast<PixelType*>(malloc(Size()));
			memcpy(m_Pixels, pixels, Size());
		}

		void Set(PixelType*&& pixels, const uint64_t new_width, const uint64_t new_height, const uint8_t new_channels) {
			Release();
			m_Height = new_height;
			m_Width = new_width;
			m_Channels = new_channels;
			m_Pixels = pixels;
		}

		void Clear() {
			if (!m_Pixels) return;
			memset(m_Pixels, 0, Count() * PixelSize);
		}

		void Release() {
			free(m_Pixels);
			m_Width = 0;
			m_Height = 0;
			m_Channels = 0;
			m_Pixels = nullptr;
		}

		void ChangeWidth(const uint64_t new_width) {
			if (!m_Pixels) return;
			if (new_width == m_Width) return;

			PixelType* new_image = reinterpret_cast<PixelType*>(calloc(new_width*m_Height*m_Channels, PixelSize));
			if (!new_image) return;

			for (uint64_t y = 0; y < m_Height; ++y) {
				for (uint64_t x = 0; x < std::min(m_Width,new_width); ++x) {
					for (uint64_t c = 0; c < m_Channels; ++c) {
						const uint64_t old_index = GetIndex(x,y,c);
						const uint64_t new_index = (y * new_width * m_Channels) + (x * m_Channels) + c;
						new_image[new_index] = m_Pixels[old_index];
					}
				}
			}

			free(m_Pixels);
			m_Pixels = new_image;
			m_Width = new_width;
		}

		void ChangeHeight(const uint64_t new_height) {
			if (!m_Pixels) return;
			if (new_height == m_Height) return;

			PixelType* new_image = reinterpret_cast<PixelType*>(calloc(m_Width*new_height*m_Channels, PixelSize));
			if (!new_image) return;

			for (uint64_t y = 0; y < std::min(new_height, m_Height); ++y) {
				for (uint64_t x = 0; x < m_Width; ++x) {
					for (uint64_t c = 0; c < m_Channels; ++c) {
						const uint64_t old_index = GetIndex(x,y,c);
						const uint64_t new_index = (y * m_Width * m_Channels) + (x * m_Channels) + c;
						new_image[new_index] = m_Pixels[old_index];
					}
				}
			}

			free(m_Pixels);
			m_Pixels = new_image;
			m_Height = new_height;
		}

		void ChangeChannels(const uint8_t new_channels) {
			if (!m_Pixels) return;
			if (new_channels == m_Channels) return;

			PixelType* new_image = reinterpret_cast<PixelType*>(calloc(m_Width*m_Height*new_channels, PixelSize));
			if (!new_image) return;

			for (uint64_t y = 0; y < m_Height; ++y) {
				for (uint64_t x = 0; x < m_Width; ++x) {
					for (uint64_t c = 0; c < std::min(m_Channels,new_channels); ++c) {
						const uint64_t old_index = GetIndex(x,y,c);
						const uint64_t new_index = (y * m_Width * new_channels) + (x * new_channels) + c;
						new_image[new_index] = m_Pixels[old_index];
					}
				}
			}

			free(m_Pixels);
			m_Pixels = new_image;
			m_Channels = new_channels;
		}

		void ChangeSize(const uint64_t new_width, const uint64_t new_height, const uint8_t new_channels) {
			if (!m_Pixels) return;
			if (new_width == m_Width &&
				new_height == m_Height &&
				new_channels == m_Channels) return;

			PixelType* new_image = calloc(new_width*new_height*new_channels, PixelSize);
			if (!new_image) return;

			for (uint64_t y = 0; y < std::min(new_height, m_Height); ++y) {
				for (uint64_t x = 0; x < std::min(new_width, m_Width); ++x) {
					for (uint64_t c = 0; c < std::min(new_channels, m_Channels); ++c) {
						const uint64_t old_index = GetIndex(x,y,c);
						const uint64_t new_index = (y * new_width * new_channels) + (x * new_channels) + c;
						new_image[new_index] = m_Pixels[old_index];
					}
				}
			}

			free(m_Pixels);
			m_Pixels = new_image;
			m_Height = new_height;
			m_Width = new_width;
			m_Channels = new_channels;
		}
	public:
		[[nodiscard]] PixelType* Get() const {
			return m_Pixels;
		}

		[[nodiscard]] PixelType* Get(const uint64_t x, const uint64_t y, const uint8_t channel) const {
			if (!m_Pixels) return nullptr;
			return &m_Pixels[GetIndex(x,y,channel)];
		}
		[[nodiscard]] PixelType& At(const uint64_t x, const uint64_t y, const uint8_t channel) const {
			if (!m_Pixels) throw std::logic_error("Image not allocated.");
			if (!Exist(x,y,channel)) throw std::logic_error("The position x, y, channel is not valid.");
			return m_Pixels[GetIndex(x,y,channel)];
		}
		[[nodiscard]] uint64_t GetIndex(const uint64_t x, const uint64_t y, const uint8_t channel) const {
			return (y * m_Width * m_Channels) + (x * m_Channels) + channel;
		}

		[[nodiscard]] uint64_t Count() const {return m_Width * m_Height * m_Pixels;}
		[[nodiscard]] uint64_t Size() const {return Count() * PixelSize;}
		[[nodiscard]] bool Exist(const uint64_t x, const uint64_t y, const uint8_t channel) const {
			return m_Pixels && x < m_Width && y < m_Height && channel < m_Channels;
		}
		[[nodiscard]] bool IsValid() const {return m_Pixels != nullptr;}

		[[nodiscard]] uint64_t GetWidth() const { return m_Width; }
		[[nodiscard]] uint64_t GetHeight() const { return m_Height; }
		[[nodiscard]] uint8_t GetChannels() const { return m_Channels; }

	public:
		[[nodiscard]] operator bool() const {return IsValid();}
		[[nodiscard]] PixelType& operator()(const uint64_t x, const uint64_t y, const uint8_t channel) const {return Get(x,y,channel);}
	private:
		PixelType* m_Pixels{nullptr};
		uint64_t m_Width{0};
		uint64_t m_Height{0};
		uint8_t m_Channels{0};
	};

} // namespace Imagine::Core
