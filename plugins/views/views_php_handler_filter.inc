<?php
// $Id$

/**
 * A handler to provide a field that is completely custom by the administrator.
 *
 * @ingroup views_filter_handlers
 */
class views_php_handler_filter extends views_handler_filter {

  /**
   * Only let users with use PHP permissions set/modify this handler.
   */
  function php_check_access(&$element) {
    if (!user_access('use PHP for settings')) {
      $element['#disabled'] = TRUE;
      $element['#value'] = $element['#default_value'];
      $element['#description'] .= ' <strong>' . t('Note: you do not have permission to modify this.') . '</strong>';
    }
  }

  /**
   * Implements views_object#option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['php_static'] = array('default' => '');
    $options['php_filter'] = array('default' => '');
    return $options;
  }

  /**
   * Implements views_handler#option_definition().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['php_static'] = array(
      '#type' => 'textarea',
      '#title' => t('Static code'),
      '#default_value' => $this->options['php_static'],
      '#rows' => 5,
      '#description' =>
        t('Code to run right before execution of the view. May be useful to define functions to be re-used in the value and/or output code.') . ' <strong>' . t('Do not include &lt;?php ?&gt; delimiters to enclose PHP code.') . '</strong><br/>' .
        t('Available variables:') . '<br/>' .
        t('$static: can be used to store reusable data per row.')
      ,
    );
    $this->php_check_access($form['php_static']);
    $form['php_filter'] = array(
      '#type' => 'textarea',
      '#title' => t('Filter code'),
      '#default_value' => $this->options['php_filter'],
      '#rows' => 5,
      '#description' =>
        t('The text that should be displayed.')
        .' '.t('Include &lt;?php ?&gt; delimiters when using PHP code.')
        .' '.t('Available variables:').'<br/>'
        .t('$data: contains the retrieved record from the database (e.g. $data->nid).').'<br/>'
        .t('$static: can be used to store reusable data per row.')
      ,
    );
    $this->php_check_access($form['php_filter']);
  }

  /**
   * Implements views_handler_filter#query().
   */
  function query() {
    // do nothing -- to override the parent query.
  }

  /**
   *
   * @see views_php_views_pre_execute()
   * @see self::php_post_execute()
   */
  function php_pre_execute() {
    // Ecexute static PHP code.
    if (!empty($this->options['php_static'])) {
      $this->php_static(&$this->php_static_variable);
    }
  }

  /**
   *
   * @see views_php_views_post_execute()
   */
  function php_post_execute() {
    // Evaluate the PHP code.
    if (!empty($this->options['php_filter'])) {
      foreach ($this->view->result as $i => $row) {
        if ($this->php_filter($row, &$this->php_static_variable)) {
          unset($this->view->result[$i]);
        }
      }
    }
  }

  /**
   *
   */
  function php_static(&$static) {
    ob_start();
    eval($this->options['php_static']);
    return ob_get_clean();
  }

  /**
   *
   */
  function php_filter($data) {
    ob_start();
    $result = eval($this->options['php_filter']);
    ob_get_clean();
    return !empty($result);
  }
}
