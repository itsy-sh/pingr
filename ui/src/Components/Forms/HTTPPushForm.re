type pingFormTypes =
  /* Push tests */
  | TestName
  | Timeout
  /* Contacts */
  | Contacts;

type pingFormState = {
  /* Push tests */
  mutable testName: Form.t,
  mutable timeout: Form.t,
  /* Contacts */
  mutable contacts: Form.t,
};

let getInitialFormState = () => {
  testName: Str(""),
  timeout: Int(0),
  contacts: TupleList([]),
};

module HTTPPushFormConfig = {
  type field = pingFormTypes;
  type state = pingFormState;
  let update = (field, value, state) => {
    switch (field, value) {
    | (TestName, v) => {...state, testName: v}
    | (Timeout, v) => {...state, timeout: v}
    | (Contacts, v) => {...state, contacts: v}
    };
  };
  let get = (field, state) => {
    switch (field) {
    | TestName => state.testName
    | Timeout => state.timeout
    | Contacts => state.contacts
    };
  };
};

module HTTPPushForm = Form.FormComponent(HTTPPushFormConfig);

let rules = [
  (TestName, [(Form.NotEmpty, FormHelpers.emptyMsg)]),
  (Timeout, [(Form.NotEmpty, FormHelpers.aboveZero)]),
  (
    Contacts,
    [
      (
        Form.Custom(FormHelpers.validContactThreshold),
        FormHelpers.thresholdMsg,
      ),
    ],
  ),
];

let getTestPayload = (~inputTest=?, values) => {
  let payload = Js.Dict.empty();
  switch (inputTest) {
  | Some(test) =>
    Models.Test.(
      switch (test) {
      | Some(test_) =>
        FormHelpers.setJsonKey(payload, "test_id", Str(test_.testId))
      | None => ()
      }
    )

  | None => ()
  };

  FormHelpers.setJsonKey(payload, "test_type", Str("HTTPPush"));
  FormHelpers.setJsonKey(payload, "test_name", values.testName);
  FormHelpers.setJsonKey(payload, "timeout", values.timeout);

  /* Lazy fix for push tests*/
  FormHelpers.setJsonKey(payload, "url", Str(""));
  FormHelpers.setJsonKey(payload, "interval", Int(0));

  let blob = Js.Dict.empty();
  Js.Dict.set(payload, "blob", Js.Json.object_(blob));

  payload;
};

[@react.component]
let make =
    (
      ~submitTest,
      ~submitContacts,
      ~inputTest: option(Models.Test.t)=?,
      ~inputTestContacts=?,
    ) => {
  let (submitted, setSubmitted) = React.useState(_ => false);
  let (submitError, setSubmitError) = React.useState(() => "");

  let rec postCallback = (values, resp) => {
    switch (resp) {
    | Api.Error(msg) => setSubmitError(_ => msg)
    | Api.SuccessJSON(testJson) =>
      let testContacts =
        switch (values.contacts) {
        | TupleList(l) => l
        | _ => []
        };
      if (List.length(testContacts) > 0) {
        let id = Models.Decode.test(testJson).testId;
        submitContacts(
          FormHelpers.getContactsPayload(id, TupleList(testContacts)),
          postCallback(values),
        );
      } else {
        Paths.goToTests();
      };
    | Success(_) => Paths.goToTests()
    };
  };

  let handleSubmit = (e, values, errors) => {
    ReactEvent.Form.preventDefault(e);
    setSubmitted(_ => true);
    if (List.length(errors) == 0) {
      let payload = getTestPayload(values, ~inputTest);
      submitTest(payload, postCallback(values));
    };
  };
  <HTTPPushForm
    rules
    initialState={
                   let init = getInitialFormState();
                   switch (inputTest) {
                   | None => ()
                   | Some(test) =>
                     init.testName = Str(test.testName);
                     init.timeout = Int(test.timeout);
                   };
                   switch (inputTestContacts) {
                   | None => ()
                   | Some(testContacts) =>
                     init.contacts =
                       TupleList(
                         testContacts
                         |> List.map(testContact => {
                              Models.TestContact.(
                                testContact.contactId,
                                string_of_int(testContact.threshold),
                              )
                            }),
                       )
                   };
                   init;
                 }
    render={(f: HTTPPushForm.form) =>
      <form
        onSubmit={e => handleSubmit(e, f.form.values, f.form.errors)}
        className="w-full">
        <div className="flex flex-wrap -mx-3 mb-6">
          <FormInput
            type_=Text
            width=Full
            label="Name"
            placeholder="Some name"
            infoText="Test's name, no functional meaning (*)"
            errorMsg={
              submitted
                ? FormHelpers.getError(TestName, f.form.errors) : React.null
            }
            value={f.form.values.testName}
            onChange={v => v |> f.handleChange(TestName)}
          />
        </div>
        <div className="flex flex-wrap -mx-3 mb-6">
          <FormInput
            type_=Number
            width=Full
            label="Timeout (s)"
            infoText="Maximum seconds between pushes before error is thrown (*)"
            errorMsg={
              submitted
                ? FormHelpers.getError(Timeout, f.form.errors) : React.null
            }
            value={f.form.values.timeout}
            onChange={v => v |> f.handleChange(Timeout)}
          />
        </div>
        <FormTestContacts
          errorMsg={
            submitted
              ? FormHelpers.getError(Contacts, f.form.errors) : React.null
          }
          value={
            switch (f.form.values.contacts) {
            | TupleList(l) => l
            | _ => []
            }
          }
          onChange={v => v |> f.handleChange(Contacts)}
        />
        <button
          type_="submit"
          className="bg-green-500 hover:bg-green-700 text-white py-1 px-2 rounded">
          {"Submit" |> React.string}
        </button>
        {submitError != ""
           ? <p className="text-red-500">
               {"Error posting test: " ++ submitError |> React.string}
             </p>
           : React.null}
        <div className="h-32" />
      </form>
    }
  />;
};