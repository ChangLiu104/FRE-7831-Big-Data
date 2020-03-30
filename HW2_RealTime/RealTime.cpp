// bb_api_cpp_main.cpp
// Example Bloomberg API C++ Program (c)2014 Richard Holowczak
// Portions (c) Bloomberg, LLC

// Bloomberg API Includes
#include <blpapi_defs.h>
#include <blpapi_correlationid.h>
#include <blpapi_element.h>
#include <blpapi_event.h>
#include <blpapi_exception.h>
#include <blpapi_message.h>
#include <blpapi_session.h> 
#include <blpapi_subscriptionlist.h>
#include <iostream>
#include <fstream>

using  namespace BloombergLP;
using  namespace blpapi;
using namespace std;

class BloombergEventHandler : public EventHandler
{
public:
	// Empty constructor
	BloombergEventHandler()
	{
	}

	// The actual event handler method
	bool processEvent(const Event &event, Session *session)
	{
		std::ofstream myfile;
		myfile.open("realtime.txt", ios_base::app);
		// We are only interested in market data events related to
		// our subscription
		if (event.eventType() == Event::SUBSCRIPTION_DATA)
		{
			// Each event may have multiple messages. So
			// create a message iterator
			MessageIterator msgIter(event);
			// Iterate over each of the messages
			while (msgIter.next())
			{
				// Get one message and store it in msg
				Message msg = msgIter.message();
				// For Debugging use the msg.print method to
				// display all of the fields:
				// msg.print(std::cout);


				// Declare a pointer to the topic of the message
				const char *topic = (char *)msg.correlationId().asPointer();
				// If the elements are present, then print out the message 
				if (msg.hasElement("LAST_TRADE", true) &&
					msg.hasElement("SIZE_LAST_TRADE", true) &&
					msg.hasElement("TRADE_UPDATE_STAMP_RT", true))
				{
					// Show the topic
					cout << topic << " Trade:	";
					// Fetch the necessary elements from the message
					cout << msg.getElementAsString("TRADE_UPDATE_STAMP_RT") << ", ";
					cout << msg.getElementAsFloat64("LAST_TRADE") << ", ";
					cout << msg.getElementAsInt32("SIZE_LAST_TRADE");
					cout << endl;
					myfile << topic << "Trade:	";
					myfile << msg.getElementAsString("TRADE_UPDATE_STAMP_RT") << ", ";
					myfile << msg.getElementAsFloat64("LAST_TRADE") << ", ";
					myfile << msg.getElementAsInt32("SIZE_LAST_TRADE") << ", ";
					myfile << endl;
				}

				if (msg.hasElement("BID", true) &&
					msg.hasElement("BID_SIZE", true) &&
					msg.hasElement("BID_UPDATE_STAMP_RT", true))
				{
					cout << topic << " BID Quote:	";
					cout << msg.getElementAsString("BID_UPDATE_STAMP_RT") << ", ";
					cout << msg.getElementAsFloat64("BID") << ", ";
					cout << msg.getElementAsInt32("BID_SIZE") * 100;
					cout << endl;
					myfile << topic << " BID Quote:	 ";
					myfile << msg.getElementAsString("BID_UPDATE_STAMP_RT") << ", ";
					myfile << msg.getElementAsFloat64("BID") << ", ";
					myfile << msg.getElementAsInt32("BID_SIZE") * 100;
					myfile << endl;
				}

				if (msg.hasElement("ASK", true) &&
					msg.hasElement("ASK_SIZE", true) &&
					msg.hasElement("ASK_UPDATE_STAMP_RT", true))
				{
					cout << topic << " ASK Quote:	";
					cout << msg.getElementAsString("ASK_UPDATE_STAMP_RT") << ", ";
					cout << msg.getElementAsFloat64("ASK") << ", ";
					cout << msg.getElementAsInt32("ASK_SIZE") * 100;
					cout << endl;
					myfile << topic << " ASK Quote:	 ";
					myfile << msg.getElementAsString("ASK_UPDATE_STAMP_RT") << ", ";
					myfile << msg.getElementAsFloat64("ASK") << ", ";
					myfile << msg.getElementAsInt32("ASK_SIZE") * 100;
					myfile << endl;
				}
			} // while loop over msgIter
		} // end if eventType is subscription
		myfile.close();
		return true;
	} // end bool processEvent
}; // end class BloombergEventHandler

int main(int argc, char* argv[])
{
	int result = 0;

	// Define a security and a field
	const char *security = "FB US Equity";

	std::vector< std::string > fields;
	fields.push_back("LAST_PRICE");
	fields.push_back("BID");
	fields.push_back("ASK");

	std::vector< std::string > options;
	// Define a pointer to our event handler based on our class
	BloombergEventHandler *sessionEventHandler;
	// Set of sessionOptions
	SessionOptions sessionOptions;
	sessionOptions.setServerHost("localhost");
	sessionOptions.setServerPort(8194);
	// Instantiate the new EventHandler object
	sessionEventHandler = new BloombergEventHandler();
	// Create a Session object using the sessionOptions
	// and event handler
	Session session(sessionOptions, sessionEventHandler);
	// Start the Session
	result = session.start();
	// Open up the Service to the market data feed
	result = session.openService("//blp/mktdata");

	// Create a list of subscriptions
	SubscriptionList subscriptions;
	// Add a subscription and create the CorrelationId on the fly
	subscriptions.add(security, fields, options,
		CorrelationId((char *)security));
	// Kick off the subscriptions
	session.subscribe(subscriptions);
	// At this point we are all done. Events should start
	// coming in and our event handler will get called one
	// time for each event.
	// The "main" thread will sit here below waiting for you
	// to type in a number and press enter.
	std::cin >> result;
	// Cancel our subscription 
	session.unsubscribe(subscriptions);
	// Stop this session
	session.stop();

} // end Main
