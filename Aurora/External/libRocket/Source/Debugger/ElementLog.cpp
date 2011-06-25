/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "ElementLog.h"
#include <Rocket/Core.h>
#include <Rocket/Core/StringUtilities.h>
#include "CommonSource.h"
#include "BeaconSource.h"
#include "LogSource.h"

namespace Rocket {
namespace Debugger {

const int DEFAULT_MAX_LOG_MESSAGES = 500;

ElementLog::ElementLog(const Core::String& tag) : Core::ElementDocument(tag)
{
	dirty_logs = false;
	beacon = NULL;
	current_beacon_level = Core::Log::LT_MAX;
	auto_scroll = true;
	message_content = NULL;
	current_index = 0;
	max_log_messages = DEFAULT_MAX_LOG_MESSAGES;

	// Set up the log type buttons.
	log_types[Core::Log::LT_ALWAYS].visible = true;
	log_types[Core::Log::LT_ALWAYS].class_name = "error";
	log_types[Core::Log::LT_ALWAYS].alert_contents = "A";
	log_types[Core::Log::LT_ALWAYS].log_category_contents = "A";

	log_types[Core::Log::LT_ERROR].visible = true;
	log_types[Core::Log::LT_ERROR].class_name = "error";
	log_types[Core::Log::LT_ERROR].alert_contents = "!";
	log_types[Core::Log::LT_ERROR].log_category_contents = "!!";
	log_types[Core::Log::LT_ERROR].button_name = "error_button";

	log_types[Core::Log::LT_ASSERT].visible = true;
	log_types[Core::Log::LT_ASSERT].class_name = "error";
	log_types[Core::Log::LT_ASSERT].alert_contents = "!";
	log_types[Core::Log::LT_ASSERT].log_category_contents = "!!";

	log_types[Core::Log::LT_WARNING].visible = true;
	log_types[Core::Log::LT_WARNING].class_name = "warning";
	log_types[Core::Log::LT_WARNING].alert_contents = "!";
	log_types[Core::Log::LT_WARNING].log_category_contents = "!!";
	log_types[Core::Log::LT_WARNING].button_name = "warning_button";

	log_types[Core::Log::LT_INFO].visible = false;
	log_types[Core::Log::LT_INFO].class_name = "info";
	log_types[Core::Log::LT_INFO].alert_contents = "i";
	log_types[Core::Log::LT_INFO].log_category_contents = "i";
	log_types[Core::Log::LT_INFO].button_name = "info_button";

	log_types[Core::Log::LT_DEBUG].visible = true;
	log_types[Core::Log::LT_DEBUG].class_name = "debug";
	log_types[Core::Log::LT_DEBUG].alert_contents = "?";
	log_types[Core::Log::LT_DEBUG].log_category_contents = "?";
	log_types[Core::Log::LT_DEBUG].button_name = "debug_button";
}

ElementLog::~ElementLog()
{
}

// Initialises the log element.
bool ElementLog::Initialise(unsigned int maxLogMessages)
{
	max_log_messages = maxLogMessages;

	SetInnerRML(log_rml);
	SetId("rkt-debug-log");

	message_content = static_cast<Controls::ElementFormControl*>( GetElementById("content") );
	if (message_content)
	{
		message_content->AddEventListener("resize", this);
		message_content->SetProperty("overflow", "scroll");
	}

	// Make handle invisible because it doesn't seem to align properly with textarea
	Core::Element* size_handle = GetElementById("size_handle");
	if (size_handle)
	{
		size_handle->SetProperty("background-color", "transparent");
		size_handle->SetProperty("color", "transparent");
	}

	Core::StyleSheet* style_sheet = Core::Factory::InstanceStyleSheetString(Core::String(common_rcss) + Core::String(log_rcss));
	if (style_sheet == NULL)
		return false;

	SetStyleSheet(style_sheet);
	style_sheet->RemoveReference();

	// Create the log beacon.
	beacon = GetContext()->CreateDocument();
	if (beacon == NULL)
		return false;

	beacon->SetId("rkt-debug-log-beacon");
	beacon->SetProperty("visibility", "visible");
	beacon->SetInnerRML(beacon_rml);

	// Remove the initial reference on the beacon.
	beacon->RemoveReference();

	Core::Element* button = beacon->GetFirstChild();
	if (button != NULL)
		beacon->GetFirstChild()->AddEventListener("click", this);

	style_sheet = Core::Factory::InstanceStyleSheetString(Core::String(common_rcss) + Core::String(beacon_rcss));
	if (style_sheet == NULL)
	{
		GetContext()->UnloadDocument(beacon);
		beacon = NULL;

		return false;
	}

	beacon->SetStyleSheet(style_sheet);
	style_sheet->RemoveReference();

	return true;
}

// Adds a log message to the debug log.
void ElementLog::AddLogMessage(Core::Log::Type type, const Core::String& message)
{
	// Prepare message for display
	const Core::String category = Core::String(32, "%s   ", log_types[type].log_category_contents.CString());
	Core::String line = category + message;
	line = line.Replace("\n", Core::String(34, "\n%s", category.CString())).Replace("<", "&lt;").Replace(">", "&gt;");
	if (line.RFind(category) == line.Length() - category.Length())
	{
		line = line.Substring(0, line.Length() - category.Length()); // Strip final category off end of line that we added in Replace() above
	}

	// Add the message to the list of messages for the specified log type.
	LogMessage log_message;
	log_message.index = current_index++;
	log_message.message = line;
	log_types[type].log_messages.push_back(log_message);
	if (log_types[type].log_messages.size() >= max_log_messages)
	{
		log_types[type].log_messages.pop_front();
	}

	// If this log type is invisible, and there is a button for this log type, then change its text from
	// "Off" to "Off*" to signal that there are unread logs.
	if (!log_types[type].visible)
	{
		if (!log_types[type].button_name.Empty())
		{
			Rocket::Core::Element* button = GetElementById(log_types[type].button_name);
			if (button)
			{
				button->SetInnerRML("Off*");
			}
		}
	}
	else
	{
		SetBeacon(type);

		// Force a refresh of the RML.
		dirty_logs = true;
	}
}

void ElementLog::SetBeacon(Core::Log::Type type)
{
	if (beacon != NULL)
	{
		Rocket::Core::Element* beacon_button = beacon->GetFirstChild();

		if (type < current_beacon_level)
		{
			current_beacon_level = type;
			if (beacon_button)
			{
				beacon_button->SetClassNames(log_types[type].class_name);
				beacon_button->SetInnerRML(log_types[type].alert_contents);
			}
		}
		else if (type == Core::Log::LT_MAX)
		{
			current_beacon_level = type;
			if (beacon_button)
			{
				beacon_button->SetClassNames("default");
				beacon_button->SetInnerRML("...");
			}
		}
	}
}

void ElementLog::OnRender()
{
	Core::ElementDocument::OnRender();

	if (dirty_logs)
	{
		// Set the log content:
		if (message_content)
		{
			Core::String text;

			unsigned int log_pointers[Core::Log::LT_MAX];
			for (int i = 0; i < Core::Log::LT_MAX; i++)
				log_pointers[i] = 0;
			int next_type = FindNextEarliestLogType(log_pointers);
			int num_messages = 0;
			while (next_type != -1)
			{
				// Append log message
				text += log_types[next_type].log_messages[log_pointers[next_type]].message;

				log_pointers[next_type]++;
				next_type = FindNextEarliestLogType(log_pointers);
				num_messages++;
			}

			message_content->SetDisabled(false);
			message_content->SetValue(text);
			message_content->SetDisabled(true);

			dirty_logs = false;
		}
	}
}

void ElementLog::ProcessEvent(Core::Event& event)
{
	Core::Element::ProcessEvent(event);

	// Only process events if we're visible
	if (beacon != NULL)
	{
		if (event == "click")
		{
			if (event.GetTargetElement() == beacon->GetFirstChild())
			{
				if (!IsVisible())
					SetProperty("visibility", "visible");

				SetBeacon(Core::Log::LT_MAX); 
			}
			else if (event.GetTargetElement()->GetId() == "close_button")
			{
				if (IsVisible())
					SetProperty("visibility", "hidden");
			}
			else
			{
				for (int i = 0; i < Core::Log::LT_MAX; i++)
				{
					if (!log_types[i].button_name.Empty() && event.GetTargetElement()->GetId() == log_types[i].button_name)
					{
						log_types[i].visible = !log_types[i].visible;
						if (log_types[i].visible)
							event.GetTargetElement()->SetInnerRML("On");
						else
							event.GetTargetElement()->SetInnerRML("Off");
						dirty_logs = true;
					}
				}
			}
		}
	}

	if (event == "resize" && auto_scroll)
	{
		if (message_content != NULL)
			message_content->SetScrollTop(message_content->GetScrollHeight());
	}
}

int ElementLog::FindNextEarliestLogType(unsigned int log_pointers[Core::Log::LT_MAX])
{
	int log_channel = -1;
	unsigned int index = UINT_MAX;

	for (int i = 0; i < Core::Log::LT_MAX; i++)
	{
		if (log_types[i].visible)
		{
			if (log_pointers[i] < log_types[i].log_messages.size())
			{
				if (log_types[i].log_messages[log_pointers[i]].index < index)
				{
					index = log_types[i].log_messages[log_pointers[i]].index;
					log_channel = i;
				}
			}
		}
	}

	return log_channel;
}

}
}
