#pragma once

#include "engine.h"

// assume an object entry is entered by a list of values (otherwise its a list/float/string)
//	we need that list of values to display/select from somehow

template <typename T>
concept TrivialEntry = requires (T a) {
	typename T::ValueType;
	{ getRepresentation(a) } -> std::same_as<std::string>;
	{ getValue(a) } -> std::same_as<typename T::ValueType>;
};

template <typename T>
concept FiniteObjectType = requires (T) {
	{ getObjectSet<T>() } -> std::same_as<std::span<T const> const>;
	std::is_copy_assignable_v<T>;
};

struct IntegerEntry {
	typedef int ValueType;
};

std::string getRepresentation(IntegerEntry const& entry);
IntegerEntry::ValueType getValue(IntegerEntry const& entry);

struct FloatEntry {
	typedef float ValueType;
};

std::string getRepresentation(FloatEntry const& entry);
FloatEntry::ValueType getValue(FloatEntry const& entry);

struct StringEntry {
	typedef std::string ValueType;
};

std::string getRepresentation(StringEntry const& entry);
StringEntry::ValueType getValue(StringEntry const& entry);

template <typename ObjectType>
struct ObjectEntry {
	typedef ObjectType ValueType;
};

template <typename ObjectType>
std::string getRepresentation(ObjectEntry<ObjectType> const& entry);

template <typename ObjectType>
ObjectEntry<ObjectType>::ValueType getValue(ObjectEntry<ObjectType> const& entry);

template <TrivialEntry ValueEntry>
struct ListEntry {
	typedef ValueEntry::Value ValueType;
};

template <TrivialEntry ValueEntry>
std::string getRepresentation(ListEntry<ValueEntry> const& entry);

template <TrivialEntry ValueEntry>
ListEntry<ValueEntry>::ValueType getValue(ListEntry<ValueEntry> const& entry);

//struct PipelineEntity {
//
//};
//
//struct DescriptorSetEntity {
//
//};
//
//struct DescriptorLayoutEntity {
//
//};
//
//struct TextureEntity {
//
//};
//
//struct BufferEntity {
//
//};
//
//struct BufferLayoutEntity {
//
//};
//
//struct ShaderEntity {
//
//};
